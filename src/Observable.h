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

#ifndef OBSERVABLE_H
#define	OBSERVABLE_H


/**
 * Abstract base class for an observable.
 *
 * Observers registers to an observable. The observable than notifies them
 * that someting has changed via their notify method.
 */
template <class subject>
class Observable
{
  // Attributes
  std::list<Observer<subject>*> observers;

  // Constructors and destructors
  protected:
    Observable() {}
  public:
    virtual ~Observable() {}

  // Public Methods
  public:
    /**
     * Adds a new observer
     */
    virtual void AddObserver(Observer<subject>* obs)
    {
      observers.push_back(obs);
    }

    /**
     * Notifies all observers
     */
    virtual void Notify(subject& changed_subject) const
    {
      for (auto it = observers.begin(); it != observers.end(); ++it) {
        (*it)->Notify(changed_subject);
      }
    }
};

#endif
