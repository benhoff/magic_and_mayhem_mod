#include "input_state.hpp"
#include <QtEndian>
#include <cstring>
InputState::~InputState(){clear();publish(false,{},{});if(mapping_)file_.unmap(mapping_);}
bool InputState::create(const QString& path){
    if(file_.isOpen())return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly) || !file_.resize(Size))return false;
    QByteArray bytes(Size,0);std::memcpy(bytes.data(),"MNMINK01",8);
    qToLittleEndian<quint32>(1,bytes.data()+8);qToLittleEndian<quint32>(Size,bytes.data()+12);
    if(file_.write(bytes)!=Size || !file_.flush())return false;
    mapping_=file_.map(0,Size);return mapping_;
}
void InputState::key(unsigned code,bool down){
    if(code>=keys_.size())return;
    auto& value=keys_[code];
    if(down){if(!(value&0x80000000u))value=((value+1)&0x7fffffffu)|0x80000000u;}
    else value&=0x7fffffffu;
}
void InputState::clear(){for(auto& key:keys_)key&=0x7fffffffu;}
void InputState::publish(bool active,QPoint position,QSize size){
    if(!mapping_)return;
    auto* words=reinterpret_cast<quint32*>(mapping_);
    const auto sequence=qFromLittleEndian(__atomic_load_n(words+4,__ATOMIC_RELAXED));
    __atomic_store_n(words+4,qToLittleEndian(sequence+1),__ATOMIC_SEQ_CST);
    const quint32 values[]={quint32(active),quint32(position.x()),quint32(position.y()),quint32(size.width()),quint32(size.height())};
    for(unsigned i=0;i<5;++i)__atomic_store_n(words+5+i,qToLittleEndian(values[i]),__ATOMIC_RELAXED);
    for(unsigned i=0;i<256;++i)__atomic_store_n(words+16+i,qToLittleEndian(keys_[i]),__ATOMIC_RELAXED);
    __atomic_store_n(words+4,qToLittleEndian(sequence+2),__ATOMIC_RELEASE);
}
