
#include <variant>



class BaseBlock {
};



class StmtBlock: public BaseBlock {
    public:
    void draw();
};

class FlowBlock: public BaseBlock {
    public:
    void draw();
};


class ExprBlock: public BaseBlock {
    public:
    void draw();
};


class EventBlock: public BaseBlock {
        public:
    void draw();
};

using Block = std::variant<StmtBlock, FlowBlock, ExprBlock, EventBlock>;
