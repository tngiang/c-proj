#include "shardkv_client.hpp"

std::optional<std::string> ShardKvClient::Get(const std::string& key) {
  // Query shardcontroller for config
  auto config = this->Query();
  if (!config) return std::nullopt;

  // find responsible server in config
  std::optional<std::string> server = config->get_server(key);
  if (!server) return std::nullopt;

  return SimpleClient{*server}.Get(key);
}

bool ShardKvClient::Put(const std::string& key, const std::string& value) {
  // Query shardcontroller for config
  auto config = this->Query();
  if (!config) return false;

  // find responsible server in config, then make Put request
  std::optional<std::string> server = config->get_server(key);
  if (!server) return false;
  return SimpleClient{*server}.Put(key, value);
}

bool ShardKvClient::Append(const std::string& key, const std::string& value) {
  // Query shardcontroller for config
  auto config = this->Query();
  if (!config) return false;

  // find responsible server in config, then make Append request
  std::optional<std::string> server = config->get_server(key);
  if (!server) return false;
  return SimpleClient{*server}.Append(key, value);
}

std::optional<std::string> ShardKvClient::Delete(const std::string& key) {
  // Query shardcontroller for config
  auto config = this->Query();
  if (!config) return std::nullopt;

  // find responsible server in config, then make Delete request
  std::optional<std::string> server = config->get_server(key);
  if (!server) return std::nullopt;
  return SimpleClient{*server}.Delete(key);
}

std::optional<std::vector<std::string>> ShardKvClient::MultiGet(
    const std::vector<std::string>& keys) {
  std::vector<std::string> values;
  values.reserve(keys.size());
  // TODO (Part B, Step 3): Implement!

  auto config = this->Query();
  if (!config) {
    return std::nullopt;
  }

  std::unordered_map<std::string, std::vector<std::string>> server_keys;
  for (const auto& key : keys) {
    std::optional<std::string> server = config->get_server(key);
    if (!server) {
      return std::nullopt;
    }
    server_keys[*server].push_back(key);
  }

  std::unordered_map<std::string, std::string> key_value_map;
  for (const auto& [server, server_keys] : server_keys) {
    auto server_values = SimpleClient{server}.MultiGet(server_keys);
    if (!server_values) {
      return std::nullopt;
    }
    for (size_t i = 0; i < server_keys.size(); ++i) {
      key_value_map[server_keys[i]] = (*server_values)[i];
    }
  }

  for (const auto& key : keys) {
    auto it = key_value_map.find(key);
    if (it != key_value_map.end()) {
      values.push_back(it->second);
    } else {
      return std::nullopt;
    }
  }

  return values;  
}

bool ShardKvClient::MultiPut(const std::vector<std::string>& keys,
                             const std::vector<std::string>& values) {
  // TODO (Part B, Step 3): Implement!

  if (keys.size() != values.size()) {
    return false;
  }

  auto config = this->Query();
  if (!config) {
    return false;
  } 

  std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> server_kv_pairs;
  for (size_t i = 0; i < keys.size(); ++i) {
    std::optional<std::string> server = config->get_server(keys[i]);
    if (!server) {
      return false;
    }
    server_kv_pairs[*server].emplace_back(keys[i], values[i]);
  }

  for (const auto& [server, kv_pairs] : server_kv_pairs) {
    std::vector<std::string> server_keys;
    std::vector<std::string> server_values;
    server_keys.reserve(kv_pairs.size());
    server_values.reserve(kv_pairs.size());

    for (const auto& [key, value] : kv_pairs) {
      server_keys.push_back(key);
      server_values.push_back(value);
    }

    bool success = SimpleClient{server}.MultiPut(server_keys, server_values);
    if (!success) {
      return false;
    } 
  }

  return true;
}

// Shardcontroller functions
std::optional<ShardControllerConfig> ShardKvClient::Query() {
  QueryRequest req;
  if (!this->shardcontroller_conn->send_request(req)) return std::nullopt;

  std::optional<Response> res = this->shardcontroller_conn->recv_response();
  if (!res) return std::nullopt;
  if (auto* query_res = std::get_if<QueryResponse>(&*res)) {
    return query_res->config;
  }

  return std::nullopt;
}

bool ShardKvClient::Move(const std::string& dest_server,
                         const std::vector<Shard>& shards) {
  MoveRequest req{dest_server, shards};
  if (!this->shardcontroller_conn->send_request(req)) return false;

  std::optional<Response> res = this->shardcontroller_conn->recv_response();
  if (!res) return false;
  if (auto* move_res = std::get_if<MoveResponse>(&*res)) {
    return true;
  }

  return false;
}
