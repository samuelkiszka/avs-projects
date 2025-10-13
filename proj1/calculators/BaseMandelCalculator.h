/**
 * @file BaseMandelCalculator.h 
 * @author Vojtech Mrazek (mrazek@fit.vutbr.cz)
 * @author Marta Jaros (jarosmarta@fit.vutbr.cz)
 * @brief Abstract class for mandelbrot calculator
 * @date 2025-10-09
 * 
 */

#ifndef BASEMANDELCALCULATOR_H
#define BASEMANDELCALCULATOR_H

#include <string>
#include <iostream>

/**
 * @brief Abstract class for Mandelbrot set calculator, calculates the dimensions
 * 
 */
class BaseMandelCalculator
{
public:
    /**
     * @brief Construct a new Base Mandel Calculator object
     * 
     * @param matrixBaseSize basic size (width will be multiplied by 3, height by 2)
     * @param limit number of iterations
     * @param cName name of the calculator
     */
    BaseMandelCalculator(unsigned matrixBaseSize, unsigned limit, const std::string & cName);
    
    /**
     * @brief Prints output to ostream 
     * 
     * @param cout output stream
     * @param batchMode true = compact CSV output
     */
    void info(std::ostream & cout, bool batchMode);
    
    int width; // width of the set
    int height; // height of the set


protected:
    const std::string cName;
    const int limit;
    bool batchMode;


    const float x_start; // minimal real value
    const float x_fin; // maximal real value
    const float y_start; // minimal imag value
    const float y_fin; // maximal imag value
        
    float dx; // step of real values
    float dy; // step of imag values
};

#endif
