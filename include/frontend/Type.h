#ifndef TYPE_H
#define TYPE_H

class Type
{
public:
    enum TypeID 
    {
        VoidTy=0,
        BoolTy,
        CharTy,
        IntegerTy,
        FloatTy,
        DoubleTy,
        ArrayTy,
        StringTy,
        StructTy
    };

    explicit Type(TypeID id) : m_typeID(id){}
    TypeID getTypeID() { return m_typeID; }
private:
    TypeID m_typeID;	
};

#endif //TYPE_H
