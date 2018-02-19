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

#include "sling/nlp/parser/parser-action.h"

#include "sling/frame/object.h"
#include "sling/string/strcat.h"

namespace sling {
namespace nlp {

string ParserAction::TypeName(Type type) {
  switch (type) {
    case ParserAction::EVOKE: return "EVOKE";
    case ParserAction::REFER: return "REFER";
    case ParserAction::CONNECT: return "CONNECT";
    case ParserAction::ASSIGN: return "ASSIGN";
    case ParserAction::EMBED: return "EMBED";
    case ParserAction::ELABORATE: return "ELABORATE";
    case ParserAction::SHIFT: return "SHIFT";
    case ParserAction::STOP: return "STOP";
  }

  return "<ERROR>";
}

string ParserAction::TypeName() const {
  return TypeName(type);
}

string ParserAction::ToString(Store *store) const {
  string s = StrCat(TypeName(), ":");
  switch (type) {
    case ParserAction::EVOKE:
      StrAppend(&s, "len=", length, ":", store->DebugString(label));
      break;
    case ParserAction::REFER:
      StrAppend(&s, "len=", length, ":", target);
      break;
    case ParserAction::CONNECT:
      StrAppend(&s, source, " -> ", store->DebugString(role), " -> ", target);
      break;
    case ParserAction::ASSIGN:
      StrAppend(&s, source, " -> ", store->DebugString(role), " -> ",
                store->DebugString(label));
      break;
    case ParserAction::EMBED:
      StrAppend(&s, "TYPE(", store->DebugString(label), ")",
                " -> ", store->DebugString(role), " -> ",
                target);
      break;
    case ParserAction::ELABORATE:
      StrAppend(&s, "TYPE(", store->DebugString(label), ")",
                " <- ", store->DebugString(role), " <- ",
                source);
      break;
    case ParserAction::SHIFT:
    case ParserAction::STOP:
    default:
      s.pop_back();
      break;
  }

  return s;
}

}  // namespace nlp
}  // namespace sling

