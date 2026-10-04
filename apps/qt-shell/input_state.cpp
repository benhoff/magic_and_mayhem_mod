#include "input_state.hpp"
#include <QtEndian>
#include <cstring>
InputState::~InputState(){clear();publish(false,{},{});if(mapping_)file_.unmap(mapping_);}
bool InputState::create(const QString& path){
    if(file_.isOpen())return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly) || !file_.resize(Size))return false;
    QByteArray bytes(Size,0);std::memcpy(bytes.data(),MNM_INPUT_V1_MAGIC,MNM_INPUT_V1_MAGIC_SIZE);
    qToLittleEndian<quint32>(MNM_INPUT_V1_VERSION,bytes.data()+MNM_INPUT_V1_VERSION_OFFSET);qToLittleEndian<quint32>(MNM_INPUT_V1_DECLARED_SIZE,bytes.data()+MNM_INPUT_V1_DECLARED_SIZE_OFFSET);
    if(file_.write(bytes)!=Size || !file_.flush())return false;
    mapping_=file_.map(0,Size);return mapping_;
}
void InputState::key(unsigned code,bool down){
    if(code>=keys_.size())return;
    auto& value=keys_[code];
    if(down){if(!(value&MNM_INPUT_V1_KEY_DOWN_MASK))value=((value+1)&MNM_INPUT_V1_KEY_GENERATION_MASK)|MNM_INPUT_V1_KEY_DOWN_MASK;}
    else value&=MNM_INPUT_V1_KEY_GENERATION_MASK;
}
void InputState::clear(){for(auto& key:keys_)key&=MNM_INPUT_V1_KEY_GENERATION_MASK;}
void InputState::publish(bool active,QPoint position,QSize size){
    if(!mapping_)return;
    auto* words=reinterpret_cast<quint32*>(mapping_);
    const auto sequence=qFromLittleEndian(__atomic_load_n(words+MNM_INPUT_V1_SEQUENCE_OFFSET/4,__ATOMIC_RELAXED));
    __atomic_store_n(words+MNM_INPUT_V1_SEQUENCE_OFFSET/4,qToLittleEndian(sequence+1),__ATOMIC_SEQ_CST);
    const quint32 values[]={quint32(active),quint32(position.x()),quint32(position.y()),quint32(size.width()),quint32(size.height())};
    for(unsigned i=0;i<5;++i)__atomic_store_n(words+MNM_INPUT_V1_ACTIVE_OFFSET/4+i,qToLittleEndian(values[i]),__ATOMIC_RELAXED);
    for(unsigned i=0;i<MNM_INPUT_V1_KEY_COUNT;++i)__atomic_store_n(words+MNM_INPUT_V1_KEYS_OFFSET/4+i,qToLittleEndian(keys_[i]),__ATOMIC_RELAXED);
    __atomic_store_n(words+MNM_INPUT_V1_SEQUENCE_OFFSET/4,qToLittleEndian(sequence+2),__ATOMIC_RELEASE);
}
