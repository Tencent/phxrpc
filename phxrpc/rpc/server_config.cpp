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

#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/stat.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>


#include "server_config.h"
#include "phxrpc/file.h"

namespace phxrpc {

static __thread struct sockaddr_in g_innerip_addr;
char * GetInnerIP()
{
	int sock_fd;
	static char addr[32] = { "" };
    char tmp_addr[32] = {""};

	struct in_addr my_addr[2];
	struct ifreq stIfreq[10];
	const char *innerip_list[] = {
		"10.0.0.0", "10.255.255.255",
        "100.64.0.0", "100.127.255.255",
		"172.16.0.0", "172.31.255.255",
		"192.168.0.0", "192.168.255.255",
		"169.254.0.0", "169.254.255.255"
	};

    memset(&g_innerip_addr, 0x0, sizeof(struct sockaddr_in));

	if (-1 == (sock_fd = socket(PF_INET, SOCK_DGRAM, 0)))
	{
		return addr;
	}       

	struct ifconf ifc;
	memset(&ifc, 0, sizeof(ifc));
	ifc.ifc_len = sizeof(stIfreq);
	ifc.ifc_req = stIfreq;
	ioctl(sock_fd, SIOCGIFCONF, &ifc);

    bool is_eth1 = false;
    bool is_get_innerip = false;

	for(size_t i=0; i < ifc.ifc_len / sizeof(ifreq); i++)
	{
		sockaddr_in staddr;
		memcpy(&staddr, &ifc.ifc_req[i].ifr_addr, sizeof(staddr));

        if(0 == strncasecmp(ifc.ifc_req[i].ifr_name, "eth1", strlen("eth1"))) {
            is_eth1 = true;
        }

        if(!is_eth1 && is_get_innerip) {
            continue;
        }

		for(size_t j=0; j < sizeof(innerip_list) / sizeof(innerip_list[0]) / 2; j++)
		{
			inet_aton(innerip_list[j*2], &my_addr[0]);
			my_addr[0].s_addr = htonl(my_addr[0].s_addr);
			inet_aton(innerip_list[j*2+1], &my_addr[1]);
			my_addr[1].s_addr = htonl(my_addr[1].s_addr);

			unsigned long lTmp = htonl(staddr.sin_addr.s_addr);
			if(lTmp >= my_addr[0].s_addr && lTmp <= my_addr[1].s_addr)
			{
                memcpy(&g_innerip_addr, &staddr, sizeof(struct sockaddr_in));
                if(is_eth1) {
                    inet_ntop(AF_INET, &staddr.sin_addr, addr, sizeof(addr));
                    close(sock_fd);
                    return addr;
                } 
                is_get_innerip = true;
                memset(tmp_addr, 0x0, sizeof(tmp_addr));
                inet_ntop(AF_INET, &staddr.sin_addr, tmp_addr, sizeof(tmp_addr));
            }
		}
        is_eth1 = false;
	}

    memcpy(addr, tmp_addr, sizeof(tmp_addr));
	close(sock_fd);
	return addr;
}


ServerConfig::ServerConfig() {
    memset(bind_ip_, 0, sizeof(bind_ip_));
    port_ = -1;
    max_threads_ = 120;
    socket_timeout_ms_ = 5000;
    memset(package_name_, 0, sizeof(package_name_)) ;
}

ServerConfig::~ServerConfig() {
}

bool ServerConfig::Read(const char * config_file) {
    Config config;
    if (!config.InitConfig(config_file)) {
        return false;
    }

    bool succ = true;
    succ &= config.ReadItem("Server", "BindIP", bind_ip_, sizeof(bind_ip_));
    succ &= config.ReadItem("Server", "Port", &port_);
    succ &= config.ReadItem("Server", "PackageName", package_name_, sizeof(package_name_));
    config.ReadItem("Server", "MaxThreads", &max_threads_, 20);
    config.ReadItem("Log", "LogDir", log_dir_, sizeof(log_dir_), "~/log");
    config.ReadItem("Log", "LogLevel", &log_level_, LOG_ERR);
    config.ReadItem("ServerTimeout", "SocketTimeoutMS", &socket_timeout_ms_, 5000);

    if (succ) {

        if(0 == strcasecmp(bind_ip_, "$innerip")) {
            snprintf(bind_ip_, sizeof(bind_ip_), "%s", GetInnerIP());
        }

        return DoRead(config);
    } else {
        log(LOG_ERR, "Config::%s key BindIP | Port | PackageName not found", __func__);
        return succ;
    }
}

bool ServerConfig :: DoRead(Config & config) {
    return true;
}

void ServerConfig::SetBindIP(const char * ip) {
    snprintf(bind_ip_, sizeof(bind_ip_), "%s", ip);
}

const char * ServerConfig::GetBindIP() const {
    return bind_ip_;
}

void ServerConfig::SetPort(int port) {
    port_ = port;
}

int ServerConfig::GetPort() const {
    return port_;
}

void ServerConfig::SetMaxThreads(int max_threads) {
    max_threads_ = max_threads;
}

int ServerConfig::GetMaxThreads() const {
    return max_threads_;
}

void ServerConfig::SetSocketTimeoutMS(int socket_timeout_ms) {
    socket_timeout_ms_ = socket_timeout_ms;
}

int ServerConfig::GetSocketTimeoutMS() const {
    return socket_timeout_ms_;
}

void ServerConfig::SetPackageName(const char * package_name) {
    strncpy(package_name_,package_name, sizeof(package_name_) - 1);
}

const char * ServerConfig :: GetPackageName() const {
    return package_name_;
}

const char * ServerConfig :: GetLogDir() const
{
    return log_dir_;
}

void ServerConfig :: SetLogLevel( int log_level )
{
    log_level_ = log_level;
}

int ServerConfig :: GetLogLevel() const
{
    return log_level_;
}

//////////////////////////////////////////////////////

HshaServerConfig :: HshaServerConfig()
    : max_connections_(800000), 
    max_queue_length_(20480), 
    fast_reject_threshold_ms_(20),
    fast_reject_adjust_rate_(5),
    io_thread_count_(3) {
        fast_reject_type_ = FASTREJECT_TYPE_RANDOM;
        qos_user_priority_cnt_ = 1024;
        qos_user_priority_elevate_percent_ = 5;
        qos_user_priority_lower_percent_ = 2;
        memset(qos_business_priority_conf_file_, 0x0, sizeof(qos_business_priority_conf_file_));
}

HshaServerConfig :: ~HshaServerConfig() {
}

bool HshaServerConfig :: DoRead(Config & config) {
    config.ReadItem("Server", "MaxConnections", &max_connections_, 800000);
    config.ReadItem("Server", "IOThreadCount", &io_thread_count_, 3);
    config.ReadItem("Server", "MaxQueueLength", &max_queue_length_, 20480);
    config.ReadItem("Server", "FastRejectThresholdMS", &fast_reject_threshold_ms_, 20);
    config.ReadItem("Server", "FastRejectAdjustRate", &fast_reject_adjust_rate_, 5);
    config.ReadItem("Server", "FastRejectType", &fast_reject_type_, FASTREJECT_TYPE_RANDOM);
    config.ReadItem("Server", "FastRejectQoSBusinessPriorityConfFile", qos_business_priority_conf_file_,
            sizeof(qos_business_priority_conf_file_), "");
    config.ReadItem("Server", "FastRejectQoSUserPriorityCnt", &qos_user_priority_cnt_, 1024);
    config.ReadItem("Server", "FastRejectQoSUserPriorityElevatePercent", &qos_user_priority_elevate_percent_, 5);
    config.ReadItem("Server", "FastRejectQoSUserPriorityLowerPercent", &qos_user_priority_lower_percent_, 2);
 
    return true;
}

void HshaServerConfig :: SetMaxConnections(const int max_connections) {
    max_connections_ = max_connections;
}

int HshaServerConfig :: GetMaxConnections() const {
    return max_connections_;
}

void HshaServerConfig :: SetMaxQueueLength(const int max_queue_length) {
    max_queue_length_ = max_queue_length;
}

int HshaServerConfig :: GetMaxQueueLength() const {
    return max_queue_length_;
}

void HshaServerConfig :: SetFastRejectThresholdMS(const int fast_reject_threshold_ms) {
    fast_reject_threshold_ms_ = fast_reject_threshold_ms;
}

int HshaServerConfig :: GetFastRejectThresholdMS() const {
    return fast_reject_threshold_ms_;
}

void HshaServerConfig :: SetFastRejectAdjustRate(const int fast_reject_adjust_rate) {
    fast_reject_adjust_rate_ = fast_reject_adjust_rate;
}

int HshaServerConfig :: GetFastRejectAdjustRate() const {
    return fast_reject_adjust_rate_;
}

void HshaServerConfig :: SetIOThreadCount(const int io_thread_count) {
    io_thread_count_ = io_thread_count;
}

int HshaServerConfig :: GetIOThreadCount() const {
    return io_thread_count_;
}
void HshaServerConfig :: SetQoSBusinessPriorityConfFile(const char * qos_business_priority_conf_file) {
    snprintf(qos_business_priority_conf_file_, sizeof(qos_business_priority_conf_file_), "%s", 
            qos_business_priority_conf_file);
}

const char * HshaServerConfig :: GetQoSBusinessPriorityConfFile() const {
    return qos_business_priority_conf_file_;
}

void HshaServerConfig :: SetUserPriorityCnt(const int qos_user_priority_cnt) {
    qos_user_priority_cnt_ = qos_user_priority_cnt;
}

int HshaServerConfig :: GetUserPriorityCnt() const {
    return qos_user_priority_cnt_;
}

void HshaServerConfig :: SetUserPriorityElevatePercent(const int qos_user_priority_elevate_percent) {
    qos_user_priority_elevate_percent_ = qos_user_priority_elevate_percent;
}

int HshaServerConfig :: GetUserPriorityElevatePercent() const {
    return qos_user_priority_elevate_percent_;
}

void HshaServerConfig :: SetUserPriorityLowerPercent(const int qos_user_priority_lower_percent) {
    qos_user_priority_lower_percent_ = qos_user_priority_lower_percent;
}

int HshaServerConfig :: GetUserPriorityLowerPercent() const {
    return qos_user_priority_lower_percent_;
}

void HshaServerConfig :: SetFastRejectType(const int fast_reject_type) {
    fast_reject_type_ = fast_reject_type;
}

int HshaServerConfig :: GetFastRejectType() const {
    return fast_reject_type_;
}



}
