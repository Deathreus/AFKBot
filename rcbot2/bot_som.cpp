/*
 *    This file is part of RCBot.
 *
 *    RCBot by Paul Murphy adapted from Botman's HPB Bot 2 template.
 *
 *    RCBot is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    RCBot is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with RCBot; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#endif
#include "bot_som.h"
#include "bot_mtrand.h"

float CSom :: m_fLearnRate = 1.0;

CSom :: CSom ( int iW, int iH, int iIn )
{       
	unsigned short id = 0;

	m_iW = iW;
	m_iH = iH;

	// neighbourhood size
	m_fNSize = (float)(int)((float)iW/2);

	for ( int i = 0; i < iH; i ++ )
	{
		for ( int j = 0; j < iW; j ++ )
			m_Neurons.push_back(new CSomNeuron(id++,iIn,j,i));
	}

	m_iEpochs = 0;
}

CSom :: ~CSom ()
{
	m_Neurons.clear();
}

CSomNeuron *CSom :: getBMU ( vector <float> *inputs )
{       
	CSomNeuron *winner = NULL;
	float bestdistance = 0;
	float dist;

	for ( unsigned int i = 0; i < m_Neurons.size(); i ++ )
	{       
		dist = m_Neurons[i]->distance(inputs);

		if ( !winner || (dist < bestdistance) )
		{
			winner = m_Neurons[i];
			bestdistance = dist;
		}
	}

	return winner;
}



void CSom :: updateAround ( vector<float> *inputs, CSomNeuron *bmu )
{
	float dist;
	float nsiz = (m_fNSize*m_fNSize);

	for ( unsigned int i = 0; i < m_Neurons.size(); i ++ )
	{
		CSomNeuron *current = m_Neurons[i];

		if ( (dist = bmu->neighbourDistance(current)) <= nsiz )
		{
			bmu->update(inputs,exp(-(dist) / (2*nsiz)));    
		}           
	}
}

CSomNeuron *CSom :: inputOne ( vector <float> *inputs )
{
	CSomNeuron *winner = getBMU(inputs);

	updateAround(inputs,winner);

	m_fNSize *= 0.75;
	m_fLearnRate *= 0.75;
	m_iEpochs++;

	return winner;
}

CSomNeuron *CSom :: input ( vector < vector <float> > *inputs )
{
	return inputOne(&((*inputs)[randomInt(0,(int)inputs->size()-1)]));
}

void CSom :: display ()
{
	//printf("\nDisplaying...\n");

	for ( unsigned int i = 0; i < m_Neurons.size(); i ++ )
	{
		//printf("%d -- ",i);
		m_Neurons[i]->displayWeights();
		//printf("\n");
	}
}

void CSomNeuron :: update ( vector<float> *inputs, float inf )
{
	float change;

	for ( unsigned int i = 0; i < inputs->size(); i ++ )
	{
		change = ((*inputs)[i] - fWeights[i]);

		fWeights[i] += (change*CSom::m_fLearnRate*inf);
	}
}

CSomNeuron :: ~CSomNeuron ()
{
	fWeights.clear();
}

CSomNeuron :: CSomNeuron ()
{
	m_iId = 0;
	return;
}

CSomNeuron :: CSomNeuron ( unsigned short iId, int iInp, int iX, int iY )
{				
	m_iX = (float)iX;
	m_iY = (float)iY;
	m_iId = iId;
	
	for ( int i = 0; i < iInp; i ++ )
		fWeights.push_back(randomFloat(0,1));
}

float CSomNeuron :: distance ( vector <float> *inputs )
{
	float dist = 0;
	float comp;
	
	for ( unsigned int i = 0; i < inputs->size(); i ++ )
	{
		comp = fWeights[i] - (*inputs)[i];
		
		dist += (comp*comp);
	}
	
	return dist;
}

vector <float> *CSomNeuron :: weights ()
{
	return &fWeights;
}

void CSomNeuron :: displayWeights ()
{
	for ( unsigned int i = 0; i < fWeights.size(); i ++ )
	{
		printf("%0.4f,",fWeights[i]);
	}
}

float CSomNeuron :: neighbourDistance ( CSomNeuron *other )
{
	float distx = getX()-other->getX();
	float disty = getY()-other->getY();
	
	return (distx*distx)+(disty*disty);
}
