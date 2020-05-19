#pragma once

#include <Arduino.h>

class stats{
    public:
        static inline float error(float parity, int n, uint16_t *buffer){
            return parity*sqrt(disp(n, buffer)/n);
        }

    private:
        static inline float disp(int n, uint16_t *buffer){
            int summ = 0;
            float mean_v = mean(n, buffer);
            for(unsigned i = 0; i < n; i++)
                summ += buffer[i] - 1;
            return summ/n;
        }

        static inline float mean(int n, uint16_t *buffer){
            float result = 0;
            for(unsigned i = 0; i < n; i++)
                result += buffer[i];
            return result/n;
        }
};