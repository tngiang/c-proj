#include "simple_kvstore.hpp"

bool SimpleKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  if (req->key.empty()) {
    return false;
  }
  mutex.lock();
  auto it = kvstore.find(req->key);
  if (it == kvstore.end()) {
    mutex.unlock();
    return false;
  }
  res->value = it->second;
  mutex.unlock();
  return true;
}

bool SimpleKvStore::Put(const PutRequest* req, PutResponse*) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  if (req->key.empty()) {
    return false;
  }
  mutex.lock();
  kvstore[req->key] = req->value;
  mutex.unlock();
  return true;
}

bool SimpleKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  if (req->key.empty()) {
    return false;
  }
  mutex.lock(); 
  kvstore[req->key] += req->value;
  mutex.unlock();
  return true;
}

bool SimpleKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  if (req->key.empty()) {
    return false;
  }
  mutex.lock();
  if(kvstore.find(req->key) == kvstore.end()){
    return false;
  }
  res->value = kvstore.at(req->key);
  kvstore.erase(req->key);
  mutex.unlock();
  return true;
}

bool SimpleKvStore::MultiGet(const MultiGetRequest* req,
                             MultiGetResponse* res) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  mutex.lock();
  for (const auto& key : req->keys) {
    if (key.empty()) {
      mutex.unlock();
      return false;
    }
    auto it = kvstore.find(key);
    if (it == kvstore.end()) {
      mutex.unlock();
      return false;
    }
  res->values.push_back(it->second);
  }
  mutex.unlock();
  return true;
}

bool SimpleKvStore::MultiPut(const MultiPutRequest* req, MultiPutResponse*) {
  // TODO (Part A, Step 1 and Step 2): Implement!
  if (req->keys.size() != req->values.size()) {
    return false;
  }
  mutex.lock();
  for (size_t i = 0; i < req->keys.size(); i++) {
    const auto& key = req->keys[i];
    const auto& value = req->values[i];
    if (key.empty()) {
      mutex.unlock();
      return false;
    }
    kvstore[key] = value;
  }
  mutex.unlock();
  return true;
}

std::vector<std::string> SimpleKvStore::AllKeys() {
  // TODO (Part A, Step 1 and Step 2): Implement!
  mutex.lock();
  std::vector<std::string> keys;
  for(auto const& pair : kvstore) {
    keys.push_back(pair.first);
  }
  mutex.unlock();
  return keys;
}
