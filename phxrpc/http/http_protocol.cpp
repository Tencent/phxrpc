/*
Tencent is pleased to support the open source community by making
PhxRPC available.
Copyright (C) 2016 THL A29 Limited, a Tencent company.
All rights reserved.

Licensed under the BSD 3-Clause License (the "License"); you may
not use this file except in compliance with the License. You may
obtain a copy of the License at

https://opensource.org/licenses/BSD-3-Clause

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing
permissions and limitations under the License.

See the AUTHORS file for names of contributors.
*/

#include "phxrpc/http/http_protocol.h"

#include <cassert>
#include <cstring>
#include <string>

#include "phxrpc/file.h"
#include "phxrpc/http/http_msg.h"
#include "phxrpc/network/socket_stream_base.h"


namespace {


using namespace std;


char *SeparateStr(char **s, const char *del) {
    char *d, *tok;

    if (!s || !*s)
        return nullptr;
    tok = *s;
    d = strstr(tok, del);

    if (d) {
        *s = d + strlen(del);
        *d = '\0';
    } else {
        *s = nullptr;
    }

    return tok;
}

void URLEncode(const char *source, char *dest, size_t length) {
    const char urlencstring[] = "0123456789abcdef";

    const char *p = source;
    char *q = dest;
    size_t n = 0;

    for (; *p && n < length; p++, q++, n++) {
        if (isalnum((int) *p)) {
            *q = *p;
        } else if (*p == ' ') {
            *q = '+';
        } else {
            if (n > length - 3) {
                q++;
                break;
            }

            *q++ = '%';
            int digit = *p >> 4;
            *q++ = urlencstring[digit];
            digit = *p & 0xf;
            *q = urlencstring[digit];
            n += 2;
        }
    }

    *q = 0;
}


}  // namespace


namespace phxrpc {


using namespace std;


void HttpProtocol::FixRespHeaders(bool keep_alive, const char *version, HttpResponse *resp) {
    char buffer[256]{0};

    // check keep alive header
    if (keep_alive) {
        if (nullptr == resp->GetHeaderValue(HttpMessage::HEADER_CONNECTION)) {
            resp->AddHeader(HttpMessage::HEADER_CONNECTION, "Keep-Alive");
        }
    }

    // check date header
    resp->RemoveHeader(HttpMessage::HEADER_DATE);
    time_t t_time = time(nullptr);
    struct tm tm_time;
    gmtime_r(&t_time, &tm_time);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z", &tm_time);
    resp->AddHeader(HttpMessage::HEADER_DATE, buffer);

    // check Server header
    resp->RemoveHeader(HttpMessage::HEADER_SERVER);
    resp->AddHeader(HttpMessage::HEADER_SERVER, "http/phxrpc");

    // use the same version
    resp->set_version(version);
}

void HttpProtocol::FixRespHeaders(const HttpRequest &req, HttpResponse *resp) {
    FixRespHeaders(req.keep_alive(), req.version(), resp);
}

int HttpProtocol::SendReqHeader(BaseTcpStream &socket, const char *method, const HttpRequest &req) {
    string url;

    if (req.GetParamCount() > 0) {
        url.append(req.uri());
        url.append("?");

        char tmp[1024]{0};
        for (size_t i = 0; i < req.GetParamCount(); i++) {
            if (i > 0)
                url.append("&");
            URLEncode(req.GetParamName(i), tmp, sizeof(tmp) - 1);
            url.append(tmp);
            url.append("=");
            URLEncode(req.GetParamValue(i), tmp, sizeof(tmp) - 1);
            url.append(tmp);
        }
    }

    socket << method << " " << (url.size() > 0 ? url.c_str() : req.uri())
            << " " << req.version() << "\r\n";

    for (size_t i{0}; req.GetHeaderCount() > i; ++i) {
        const char *name{req.GetHeaderName(i)};
        const char *val{req.GetHeaderValue(i)};

        socket << name << ": " << val << "\r\n";
    }

    if (0 < req.content().size()) {
        if (nullptr == req.GetHeaderValue(HttpMessage::HEADER_CONTENT_LENGTH)) {
            socket << HttpMessage::HEADER_CONTENT_LENGTH << ": "
                    << req.content().size() << "\r\n";
        }
    }

    socket << "\r\n";

    if (0 == req.content().size()) {
        if (socket.flush().good()) {
            return 0;
        } else {
            return static_cast<int>(socket.LastError());
        }
    }

    return 0;
}

int HttpProtocol::RecvRespStartLine(BaseTcpStream &socket, HttpResponse *resp) {
    char line[1024]{0};

    bool is_good{socket.getlineWithTrimRight(line, sizeof(line)).good()};
    if (is_good) {
        if (0 == strncasecmp(line, "HTTP", strlen("HTTP"))) {
            char *pos = line;
            char *first = SeparateStr(&pos, " ");
            char *second = SeparateStr(&pos, " ");

            if (nullptr != first)
                resp->set_version(first);
            if (nullptr != second)
                resp->set_status_code(atoi(second));
            if (nullptr != pos)
                resp->set_reason_phrase(pos);
        } else {
            is_good = false;
            phxrpc::log(LOG_WARNING, "WARN: Invalid response <%s>, ignored", line);
        }
    }

    if (is_good) {
        return 0;
    } else {
        phxrpc::log(LOG_WARNING, "%s, fail", __func__);
        return static_cast<int>(socket.LastError());
    }
}

int HttpProtocol::RecvReqStartLine(BaseTcpStream &socket, HttpRequest *req) {
    char line[1024]{0};

    bool is_good = socket.getlineWithTrimRight(line, sizeof(line)).good();
    if (is_good) {
        char *pos = line;
        char *first = SeparateStr(&pos, " ");
        char *second = SeparateStr(&pos, " ");

        if (nullptr != first)
            req->set_method(first);
        if (nullptr != second)
            req->set_uri(second);
        if (nullptr != pos)
            req->set_version(pos);
    } else {
        //phxrpc::log(LOG_WARNING, "WARN: Invalid request <%s>, ignored", line);
    }

    if (is_good) {
        return 0;
    } else {
        return static_cast<int>(socket.LastError());
    }
}

int HttpProtocol::RecvHeaders(BaseTcpStream &socket, HttpMessage *msg) {
    bool is_good{false};

    char *line = (char *)malloc(MAX_RECV_LEN);
    assert(nullptr != line);

    string multi_line;
    char *pos{nullptr};

    do {
        is_good = socket.getlineWithTrimRight(line, MAX_RECV_LEN).good();
        if (!is_good)
            break;

        if ((!isspace(*line)) || '\0' == *line) {
            if (multi_line.size() > 0) {
                char * header = (char*) multi_line.c_str();
                pos = header;
                SeparateStr(&pos, ":");
                for (; nullptr != pos && '\0' != *pos && isspace(*pos);)
                    pos++;
                msg->AddHeader(header, nullptr == pos ? "" : pos);
            }
            multi_line.clear();
        }

        for (pos = line; '\0' != *pos && isspace(*pos);)
            pos++;
        if ('\0' != *pos)
            multi_line.append(pos);
    } while (is_good && '\0' != *line);

    free(line);
    line = nullptr;

    if (is_good) {
        return 0;
    } else {
        return static_cast<int>(socket.LastError());
    }
}

int HttpProtocol::RecvBody(BaseTcpStream &socket, HttpMessage *msg) {
    bool is_good{true};

    const char *encoding{msg->GetHeaderValue(HttpMessage::HEADER_TRANSFER_ENCODING)};

    char *buff{(char *)malloc(MAX_RECV_LEN)};
    assert(nullptr != buff);

    if (nullptr != encoding && 0 == strcasecmp(encoding, "chunked")) {
        // read chunked, refer to rfc2616 section[19.4.6]

        for (; is_good;) {
            is_good = socket.getline(buff, MAX_RECV_LEN).good();
            if (!is_good)
                break;

            int size{static_cast<int>(strtol(buff, nullptr, 16))};
            if (size > 0) {
                for (; size > 0;) {
                    int read_len = size > MAX_RECV_LEN ? MAX_RECV_LEN : size;
                    is_good = socket.read(buff, read_len).good();
                    if (is_good) {
                        size -= read_len;
                        msg->AppendContent(buff, read_len);
                    } else {
                        break;
                    }
                }
                is_good = socket.getline(buff, MAX_RECV_LEN).good();
            } else {
                break;
            }
        }
    } else {
        const char *content_length{msg->GetHeaderValue(HttpMessage::HEADER_CONTENT_LENGTH)};

        if (nullptr != content_length) {
            int size{atoi(content_length)};

            for (; size > 0 && is_good;) {
                int read_len = size > MAX_RECV_LEN ? MAX_RECV_LEN : size;
                is_good = socket.read(buff, read_len).good();
                if (is_good) {
                    size -= read_len;
                    msg->AppendContent(buff, read_len);
                } else {
                    break;
                }
            }
        } else if (HttpMessage::Direction::RESPONSE == msg->direction()) {
            // hasn't Content-Length header, read until socket close
            for (; is_good;) {
                is_good = socket.read(buff, MAX_RECV_LEN).good();
                if (socket.gcount() > 0) {
                    msg->AppendContent(buff, socket.gcount());
                }
            }
            if (socket.eof())
                is_good = true;
        }
    }

    free(buff);

    if (is_good) {
        return 0;
    } else {
        return static_cast<int>(socket.LastError());
    }
}

int HttpProtocol::RecvReq(BaseTcpStream &socket, HttpRequest *req) {
    int ret{RecvReqStartLine(socket, req)};

    if (0 == ret)
        ret = RecvHeaders(socket, req);

    if (0 == ret)
        ret = RecvBody(socket, req);

    return ret;
}

int HttpProtocol::RecvResp(BaseTcpStream &socket, HttpResponse *resp) {
    int ret{RecvRespStartLine(socket, resp)};
    if (0 == ret)
        ret = RecvHeaders(socket, resp);

    if (0 == ret && SC_NOT_MODIFIED != resp->status_code()) {
        ret = RecvBody(socket, resp);
    }

    return ret;
}


}  // namespace phxrpc

