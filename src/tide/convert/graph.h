#include "dtypes.h"

#include "utilities/object.h"

namespace lython {

enum class PinDirection {
    Input,
    Output,
};

enum class PinKind {
    Exec,
    Data,
};

class GraphNodePinBase: public GCObject {
public:
    virtual ~GraphNodePinBase() {}


    // Can hold a value or be a link to a pin
    
    virtual String& name() = 0;
    virtual String& type() = 0;
    virtual PinDirection& direction() = 0;
    virtual PinKind& kind() = 0;
    virtual Array<GraphNodePinBase*> pins() = 0;
};

class GraphNodeBase: public GCObject {
public:
    virtual ~GraphNodeBase() {}

    virtual Array<GraphNodePinBase*>& pins() = 0;
    virtual Point<int> position() = 0;
    virtual String& comment() = 0;
};


class GraphNode: public GraphNodeBase 
{
public:
    GraphNode(Node* node = nullptr):
        _node(node)
    {}

    Array<GraphNodePinBase*>& pins() override { return _pins; };
    Point<int> position() override { return _position; };
    String& comment() override { return _comment; };
    Node*& node() { return _node; }

private:
    Node* _node;
    Array<GraphNodePinBase*> _pins;
    Point<int> _position;
    String _comment;
};

class GraphNodePin: public GraphNodePinBase 
{
public:
    

    PinDirection& direction() override { return _direction;};
    PinKind& kind() override { return _kind; };

    String& name() override { return _name; };
    String& type() override { return _type; };
    Array<GraphNodePinBase*> pins() override { return _pins; };

private:
    PinDirection _direction;
    PinKind _kind;

    String _name;
    String _type;
    Array<GraphNodePinBase*> _pins;

};

template<typename Node>
class GraphNodeAdapter: public GraphNodeBase {};

template<typename Link>
class GraphNodePinAdapter: public GraphNodePinBase {};

}