// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "sling/nlp/parser/trainer/document-batch.h"

#include "sling/frame/object.h"
#include "sling/frame/serialization.h"
#include "sling/nlp/document/document.h"

namespace sling {
namespace nlp {

void DocumentBatch::SetData(const std::vector<string> &data) {
  items_.clear();  // deallocate any existing items
  items_.resize(data.size());
  for (int i = 0; i < data.size(); ++i) {
    auto &item = items_[i];
    item.encoded = data[i];
    item.document = nullptr;
  }
}

const std::vector<string> DocumentBatch::GetSerializedData() const {
  std::vector<string> output;
  output.resize(size());
  for (int i = 0; i < size(); ++i) {
    CHECK(items_[i].document != nullptr);
    output[i] = Encode(items_[i].document->top());
  }
  return output;
}

void DocumentBatch::Decode(Store *global, bool clear_existing_annotations) {
  Handle h_mention = global->Lookup("/s/document/mention");
  Handle h_theme = global->Lookup("/s/document/theme");

  if (clear_existing_annotations) {
    CHECK(!h_mention.IsNil());
    CHECK(!h_theme.IsNil());
  }
  for (int i = 0; i < size(); ++i) {
    if (items_[i].store != nullptr) continue;

    items_[i].store = new Store(global);
    if (items_[i].encoded.empty()) {
      items_[i].document = new Document(items_[i].store);
    } else {
      StringDecoder decoder(items_[i].store, items_[i].encoded);
      Object top = decoder.Decode();
      CHECK(!top.invalid());
      items_[i].document = new Document(top.AsFrame());
      if (clear_existing_annotations) {
        Document *original = items_[i].document;
        Builder b(items_[i].store);
        for (const Slot &s : original->top()) {
          if (s.name != Handle::id() &&
              s.name != h_mention &&
              s.name != h_theme) {
            b.Add(s.name, s.value);
          }
        }
        items_[i].document = new Document(b.Create());
        delete original;
      }
    }
  }
}

}  // namespace nlp
}
