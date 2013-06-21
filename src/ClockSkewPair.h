/**
 * Copyright (C) 2013 Barbora Frankova <xfrank08@stud.fit.vutbr.cz>
 *                    Libor Polcak <ipolcak@fit.vutbr.cz>
 * 
 * This file is part of pcf - PC fingerprinter.
 *
 * Pcf is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pcf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pcf. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLOCKSKEWPAIR_H
#define	CLOCKSKEWPAIR_H

class ClockSkewPair {
public:
    /// Clock skew pair (alpha, beta): y = alpha*x + beta
    double Alpha;
    double Beta;
    ClockSkewPair(){};
    ClockSkewPair(double a, double b){
        Alpha = a;
        Beta = b;
    };
};

#endif
