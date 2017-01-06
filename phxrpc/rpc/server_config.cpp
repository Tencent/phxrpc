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

#include "server_config.h"
#include "phxrpc/file.h"

namespace phxrpc {

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
    io_thread_count_(3),
    worker_uthread_count_(0),
    worker_uthread_stack_size_(64 * 1024) {
}

HshaServerConfig :: ~HshaServerConfig() {
}

bool HshaServerConfig :: DoRead(Config & config) {
    config.ReadItem("Server", "MaxConnections", &max_connections_, 800000);
    config.ReadItem("Server", "IOThreadCount", &io_thread_count_, 3);
    config.ReadItem("Server", "WorkerUThreadCount", &worker_uthread_count_, 0);
    config.ReadItem("Server", "WorkerUThreadStackSize", &worker_uthread_stack_size_, 64 * 1024);
    config.ReadItem("Server", "MaxQueueLength", &max_queue_length_, 20480);
    config.ReadItem("Server", "FastRejectThresholdMS", &fast_reject_threshold_ms_, 20);
    config.ReadItem("Server", "FastRejectAdjustRate", &fast_reject_adjust_rate_, 5);
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

void HshaServerConfig :: SetWorkerUThreadCount(const int worker_uthread_count) {
    worker_uthread_count_ = worker_uthread_count;
}

int HshaServerConfig :: GetWorkerUThreadCount() const {
    return worker_uthread_count_;
}

void HshaServerConfig :: SetWorkerUThreadStackSize(const int worker_uthread_stack_size)
{
    worker_uthread_stack_size_ = worker_uthread_stack_size;
}

int HshaServerConfig :: GetWorkerUThreadStackSize() const
{
    return worker_uthread_stack_size_;
}

}
