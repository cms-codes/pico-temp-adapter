#ifndef TEMP_H
#define TEMP_H

class Sensor {
    public:
      Sensor() {};
      virtual float Get_value() = 0;
      virtual void Update() = 0;
    private:
      float read_value_();

      Sensor *clipper_;
      float reading = 0.0f;
};

template <class ConnType>
class Temp : public Sensor {
    public:
     Temp(ConnType *&conn_ptr) : conn_(conn_ptr) {}; 
     float Get_value() override;
     void Update() override;

    private:
     float read_temp_();

     ConnType *conn_;
     float reading_ = 0.0f;

};

#endif