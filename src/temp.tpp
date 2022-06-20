#include "temp.h"

template <class ConnType>
void Temp<ConnType>::Update()
{
    this->reading_ = this->conn_->Read_temp();
}

template <class ConnType>
float Temp<ConnType>::Get_value()
{
    return this->reading_;
}