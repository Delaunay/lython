#pragma once

#include "dtypes.h"

#include "ast/nodes.h"
#include "utilities/object.h"


#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>


namespace lython {

enum class PinDirection {
    Input,
    Output,
};

enum class PinKind {
    Flow,
    Circle,
    Square,
    Grid,
    RoundSquare,
    Diamond,
};

enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    Object,
    Delegate,
};

uint64_t nextid();

struct Base {
    uint64_t id;

    Base(): id(nextid()) {}
};

class GraphNodePinBase: public GCObject, public Base {
public:
    virtual ~GraphNodePinBase() {}

    // Can hold a value or be a link to a pin
    
    virtual String& name() = 0;
    virtual PinType& type() = 0;
    virtual PinDirection& direction() = 0;
    virtual PinKind& kind() = 0;
    virtual Array<GraphNodePinBase*> pins() = 0;

    virtual ImVec2& position() = 0;
    virtual bool connected() = 0;
};

class GraphNodeBase: public GCObject, public Base {
public:
    virtual ~GraphNodeBase() {}

    virtual Array<GraphNodePinBase*>& pins() = 0;
    virtual String& comment() = 0;
    virtual ImVec2 & size() = 0;
    virtual ImVec2& position() = 0;
    virtual bool& selected()  = 0;
     virtual String& name() = 0;

    virtual ImVec2& input_position() = 0;
    virtual ImVec2& output_position() = 0;

    void get_pins_by_direction(Array<GraphNodePinBase*>& input, Array<GraphNodePinBase*>& output) {
        for(GraphNodePinBase* pin: pins()) {
            if (pin->direction() == PinDirection::Input) {
                input.push_back(pin);
            } else {
                output.push_back(pin);
            }
        }
    }
};


class GraphNode: public GraphNodeBase
{
public:
    GraphNode(Node* node = nullptr):
        _node(node)
    {}

    Array<GraphNodePinBase*>& pins() override { return _pins; };
    ImVec2& position() override { return _position; };
    ImVec2& size() override { return _size; };
    String& comment() override { return _comment; };
    Node*& node() { return _node; }
    bool& selected() override  { return _selected; }

    ImVec2& input_position() override { return _input;};
    ImVec2& output_position() override { return _output;};
    String& name() override { return _name; }

private:
    Node* _node;
    String _name;
    Array<GraphNodePinBase*> _pins;
    ImVec2 _position;
    ImVec2 _size;
    ImVec2 _input;
    ImVec2 _output;
    String _comment;
    bool _selected;
};

class GraphNodePin: public GraphNodePinBase 
{
public:
    

    PinDirection& direction() override { return _direction;};
    PinKind& kind() override { return _kind; };

    String& name() override { return _name; };
    PinType& type() override { return _type; };
    Array<GraphNodePinBase*> pins() override { return _pins; };
    ImVec2& position() override { return _position; }
    bool connected() override { return _pins.size() > 0;}

private:
    PinDirection _direction;
    PinKind _kind;

    ImVec2 _position;
    String _name;
    PinType _type;
    Array<GraphNodePinBase*> _pins;

};

template<typename Node>
class GraphNodeAdapter: public GraphNodeBase {};

template<typename Link>
class GraphNodePinAdapter: public GraphNodePinBase {};

}