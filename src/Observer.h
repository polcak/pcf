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

#ifndef _OBSERVER_H
#define _OBSERVER_H

// This file specifies classes related to implemented observer design pattern

#include <list>

/**
 * Abstract base class for an observer.
 *
 * Observer registers to an observable and is later informed that a subject
 * has changed via the notify method.
 */
template <class subject>
class Observer
{
  // Constructors and destructors
  protected:
    Observer() {}
  public:
    virtual ~Observer() {}

  // Public methods
  public:
    virtual void Notify(std::string activity, subject& changed_subject) = 0;
};


#endif
