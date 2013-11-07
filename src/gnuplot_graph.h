/**
 * Copyright (C) 2012 Libor Polčák <ipolcak@fit.vutbr.cz>
 *                    Barbora Frankova <xfrank08@stud.fit.vutbr.cz>
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

#ifndef _GNUPLOT_GRAPH_H
#define _GNUPLOT_GRAPH_H

#include "TimeSegment.h"
#include "AnalysisInfo.h"
#include "Observer.h"
#include <string>

class gnuplot_graph: public Observer<const AnalysisInfo>
{
  private:
    std::string type;
    void generate_graph(const AnalysisInfo& changed_skew);
  
    public:
    gnuplot_graph(std::string type){
        this->type = type;
    };
    virtual void Notify(std::string none, const AnalysisInfo& changed_skew);
    std::string getOutputDirectory(){
        return type + "/";
    }
    

};

#endif
