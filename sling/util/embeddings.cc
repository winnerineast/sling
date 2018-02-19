// Copyright 2017 Google Inc. All Rights Reserved.
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

#include "sling/util/embeddings.h"

#include "sling/base/logging.h"

namespace sling {

EmbeddingReader::EmbeddingReader(const string &filename)
    : stream_(filename), input_(&stream_) {
  // Read first line with vocabulary size and embedding dimensions.
  string str;
  NextWord(&str);
  num_words_ = std::stoi(str);
  NextWord(&str);
  dim_ = std::stoi(str);
  current_word_ = 0;
  embedding_.resize(dim_);
}

bool EmbeddingReader::Next() {
  // Check if all words have been read.
  if (current_word_ == num_words_) return false;

  // Read word.
  NextWord(&word_);

  // Read embedding vector.
  char *data = reinterpret_cast<char *>(embedding_.data());
  CHECK(input_.Read(data, dim_ * sizeof(float)));

  // Read newline.
  char ch;
  CHECK(input_.Next(&ch));
  CHECK_EQ(ch, '\n');

  current_word_++;
  return true;
}

void EmbeddingReader::NextWord(string *output) {
  output->clear();
  for (;;) {
    char ch;
    CHECK(input_.Next(&ch));
    if (ch == ' ' || ch == '\n') break;
    output->push_back(ch);
  }
}

EmbeddingWriter::EmbeddingWriter(const string &filename,
                                 int num_words, int dim)
    : stream_(filename), output_(&stream_) {
  // Write header line.
  output_.Write(std::to_string(num_words));
  output_.WriteChar(' ');
  output_.Write(std::to_string(dim));
  output_.WriteChar('\n');
}

bool EmbeddingWriter::Close() {
  output_.Flush();
  return stream_.Close();
}

void EmbeddingWriter::Write(const string &word,
                            const std::vector<float> &embedding) {
  // Write word.
  output_.Write(word);
  output_.WriteChar(' ');

  // Write embedding vector.
  const char *data = reinterpret_cast<const char *>(embedding.data());
  int size = embedding.size() * sizeof(float);
  output_.Write(data, size);
  output_.WriteChar('\n');
}

}  // namespace sling

