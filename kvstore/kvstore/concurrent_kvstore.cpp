#include "concurrent_kvstore.hpp"

#include <mutex>
#include <optional>

bool ConcurrentKvStore::Get(const GetRequest* req, GetResponse* res) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  if (req->key.empty()) {
    return false;
  }
  int bucket = store.bucket(req->key); // get bucket index for given key
  store.mutex[bucket].lock_shared();
  std::optional<DbItem> item = store.getIfExists(bucket, req->key); 
  if (item == std::nullopt) {
    store.mutex[bucket].unlock_shared(); // unlock mutex if item doesn't exist
    return false;
  }
  res->value = item->value;
  store.mutex[bucket].unlock_shared();
  return true;
}

bool ConcurrentKvStore::Put(const PutRequest* req, PutResponse*) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  if (req->key.empty() || req->value.empty()) {
    return false;
  }
  int bucket = store.bucket(req->key);
  store.mutex[bucket].lock();
  store.insertItem(bucket, req->key, req->value); // insert kv-pair into bucker
  store.mutex[bucket].unlock();
  return true;
}

bool ConcurrentKvStore::Append(const AppendRequest* req, AppendResponse*) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  if (req->key.empty() || req->value.empty()) {
    return false;
  }
  int bucket = store.bucket(req->key);
  store.mutex[bucket].lock();
  std::optional<DbItem> item = store.getIfExists(bucket, req->key);
  if (item != std::nullopt) {
    store.insertItem(bucket, req->key, item->value + req->value);
  } else {
    store.insertItem(bucket, req->key, req->value);
  }
  store.mutex[bucket].unlock();
  return true;
}

bool ConcurrentKvStore::Delete(const DeleteRequest* req, DeleteResponse* res) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  if (req->key.empty()) {
    return false;
  }
  int bucket = store.bucket(req->key);
  store.mutex[bucket].lock();
  std::optional<DbItem> item = store.getIfExists(bucket, req->key);
  if (item == std::nullopt) {
    store.mutex[bucket].unlock();
    return false;
  }
  res->value = item->value;
  store.removeItem(bucket, req->key);
  store.mutex[bucket].unlock();
  return true;
}

bool ConcurrentKvStore::MultiGet(const MultiGetRequest* req,
                                 MultiGetResponse* res) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  std::vector<int> buckets; // stores bucket indices
  for (const auto& key : req->keys) {
    int bucket = store.bucket(key);
    if (std::find(buckets.begin(), buckets.end(), bucket) == buckets.end()) {
      buckets.push_back(bucket); 
    }
  }
  std::sort(buckets.begin(), buckets.end()); 
  for (int bucket : buckets) {
    store.mutex[bucket].lock_shared();
  }
  for (const auto& key : req->keys) {
    if (key.empty()) {
      for (int bucket : buckets) {
        store.mutex[bucket].unlock_shared();
      }
      return false;
    }
    int bucket = store.bucket(key);
    std::optional<DbItem> item = store.getIfExists(bucket, key);
    if (item == std::nullopt) {
      for (int bucket : buckets) {
        store.mutex[bucket].unlock_shared();
      }
      return false;
    }
    res->values.push_back(item->value);
  }
  for (int bucket : buckets) {
    store.mutex[bucket].unlock_shared();
  }
  return true;
}

bool ConcurrentKvStore::MultiPut(const MultiPutRequest* req,
                                 MultiPutResponse*) {
  // TODO (Part A, Step 3 and Step 4): Implement!
  if (req->keys.size() != req->values.size()) {
    return false;
  }
  std::vector<int> buckets;
  for (const auto& key : req->keys) {
    int bucket = store.bucket(key);
    if (std::find(buckets.begin(), buckets.end(), bucket) == buckets.end()) {
      buckets.push_back(bucket);
    }
  }
  std::sort(buckets.begin(), buckets.end());
  for (int bucket : buckets) {
    store.mutex[bucket].lock();
  }
  for (size_t i = 0; i < req->keys.size(); i++) { // iterate over kv pairs
    const auto& key = req->keys[i];
    const auto& value = req->values[i];
    if (key.empty() || value.empty()) {
      for (int bucket : buckets) {
        store.mutex[bucket].unlock();
      }
      return false;
    }
    int bucket = store.bucket(key); // get bucket index for key
    store.insertItem(bucket, key, value);
  }
  for (int bucket : buckets) {
    store.mutex[bucket].unlock();
  }
  return true;
}

std::vector<std::string> ConcurrentKvStore::AllKeys() {
  // TODO (Part A, Step 3 and Step 4): Implement!
  std::vector<std::string> keys; // vector to store all keys
  for (int i = 0; i < (int) store.BUCKET_COUNT; i++) { // iterate over all buckets
    store.mutex[i].lock_shared();
    for (const auto& item : store.buckets[i]) { // iterate over items in bucket
      keys.push_back(item.key); // add key to keys vector
    }
    store.mutex[i].unlock_shared();
  }
  return keys;
}
