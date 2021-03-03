#pragma once

#include <memory>

#include "scan_iterator.hpp"
#include "parse_iterator.hpp"

#include "node_ptr.hpp"
#include "node_vec.hpp"

#include "attribute.hpp"

#include "../pretty_printer.hpp"

namespace html {
  namespace node {
    class Base {
    public:
      virtual ~Base() {};

      virtual NodePtr parse(ParseIterator& p) const = 0;

      virtual bool closes(const std::string _candidate_name) const;

      virtual std::string inspect() const = 0;
      virtual std::string to_text() const = 0;
      virtual void pretty(PrettyPrinter& printer) const = 0;

      static NodePtr parse_document(ScanIterator& s);

    protected:
      void assert_parsing_self(ParseIterator& pi) const;
    };

    class Null : public Base {
    public:
      Null();
      ~Null() override {};

      NodePtr parse(ParseIterator& p) const override;

      std::string inspect() const override { return "Null()"; }
      std::string to_text() const override { return ""; }
      void pretty(PrettyPrinter& printer) const override { return; }
    };

    class Text : public Base {
    public:
      Text(ScanIterator& scan);

      virtual ~Text() override {};

      NodePtr parse(ParseIterator& p) const override;

      std::string inspect() const override;
      std::string to_text() const override;
      void pretty(PrettyPrinter& printer) const override;

      std::string content;

    private:
      void decode_character_reference(ScanIterator& scan);
    };

    class Tag : public Base {

    public:
      Tag() : name("_invalid_"), attribute_text("") {}
      Tag(ScanIterator& scan);

      virtual ~Tag() override {};

      NodePtr parse(ParseIterator& p) const override;

      std::string inspect() const override;
      std::string to_text() const override;
      void pretty(PrettyPrinter& printer) const override;

      std::string name;
      bool is_self_closing = false;

      std::vector<Attribute> attributes();

    private:
      NodePtr parse_normal_element(ParseIterator& p) const;
      NodePtr parse_void_element(ParseIterator& p) const;

      std::string attribute_text;

      std::vector<Attribute> _attributes;
      bool did_parse_attributes = false;
    };

    class CloseTag : public Base {
    public:
      CloseTag() : name("_invalid_") {}
      CloseTag(ScanIterator& scan);

      virtual ~CloseTag() override {};

      bool closes(const std::string candidate_name) const override;

      NodePtr parse(ParseIterator& _p) const override;

      std::string inspect() const override;
      std::string to_text() const override;
      void pretty(PrettyPrinter& printer) const override;

      std::string name;
    };

    class Element : public Base {
    public:
      Element(ParseIterator& parse);

      NodePtr parse(ParseIterator& _p) const override;

      virtual ~Element() override {};

      std::string inspect() const override;
      std::string to_text() const override;
      void pretty(PrettyPrinter& printer) const override;

      Tag opener;
      CloseTag closer;
      NodeVec _children;

    };

    class VoidElement : public Base {
    public:
      VoidElement(ParseIterator& parse);

      NodePtr parse(ParseIterator& _p) const override;

      ~VoidElement() override {};

      std::string inspect() const override;
      std::string to_text() const override;
      void pretty(PrettyPrinter& printer) const override;

      Tag opener;
    };

    class RootElement : public Base {
    public:
      RootElement(ParseIterator& parse);

      virtual ~RootElement() override {};

      NodePtr parse(ParseIterator& _p) const override;

      std::string inspect() const override;
      std::string to_text() const override;
      void pretty(PrettyPrinter& printer) const override;

      NodeVec _children;
    };

    template <typename T>
    NodePtr mks(T node) {
      return std::make_shared<T>(node);
    }
  }
}
