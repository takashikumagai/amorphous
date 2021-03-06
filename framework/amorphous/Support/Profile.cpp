/* Copyright (C) Steve Rabin, 2000. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Steve Rabin, 2000"
 */


#include "Profile.hpp"
//#include "custom_time.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <memory>

#include "amorphous/Support/Timer.hpp"


namespace amorphous
{

using namespace std;


// TODO: move this to some "win32 platform" module
class CProfileTimer_Win32 : public CProfileTimer
{
	Timer m_Timer;

public:

	CProfileTimer_Win32() {}// m_Timer.Start(); }

	~CProfileTimer_Win32() {}

	double GetExactTime() { return m_Timer.GetElapsedTimeInSeconds(); }

	double GetFrameTime() { return m_Timer.GetFrameTime(); }

	void UpdateFrameTime() { m_Timer.UpdateFrameTime(); }
};


class ProfileSample
{
public:
	bool bValid;					///< Whether this data is valid
	unsigned int iProfileInstances;	///< # of times ProfileBegin called
	int iOpenProfiles;				///< # of times ProfileBegin w/o ProfileEnd
	char szName[256];				///< Name of sample
	float fStartTime;				///< The current open profile start time
	float fAccumulator;				///< All samples this frame added together
	float fChildrenSampleTime;		///< Time taken by all children
	unsigned int iNumParents;		///< Number of profile parents

	ProfileSample() : bValid(false), iProfileInstances(0), iOpenProfiles(0), fStartTime(0), fAccumulator(0), fChildrenSampleTime(0), iNumParents(0) {}
};


class ProfileSampleHistory
{
public:
	bool bValid;        ///< Whether the data is valid
	char szName[256];   ///< Name of the sample
	float fAve;         ///< Average time per frame (percentage)
	float fMin;         ///< Minimum time per frame (percentage)
	float fMax;         ///< Maximum time per frame (percentage)

	ProfileSampleHistory() : bValid(false), fAve(0), fMin(0), fMax(0) {}
};



#define NUM_PROFILE_SAMPLES 50


/// static global variable
static ProfileSample g_samples[NUM_PROFILE_SAMPLES];
static ProfileSampleHistory g_history[NUM_PROFILE_SAMPLES];
static float g_startProfile = 0.0f;
static float g_endProfile = 0.0f;

static bool g_ProfileEnabled = true;
static bool g_ProfileRequestedState = true;

static vector<string> g_vecstrProfileText;

static std::shared_ptr<CProfileTimer> g_pTimer;


const vector<string>& GetProfileText()
{
	return g_vecstrProfileText;
}


int GetNumProfileTextRows()
{
   for( int i=0; i<NUM_PROFILE_SAMPLES; i++ )
   {
	   if( g_vecstrProfileText[i] == "" )
		   return i;
   }

   return NUM_PROFILE_SAMPLES;
}

/*
void SetProfileTimer( CProfileTimer* pTimer )
{
	SafeDelete( g_pTimer );
	g_pTimer = pTimer;
}
*/

static float GetExactTime()
{
//	static Timer s_Timer;
//	return (float)s_Timer.GetTime();

	static int s_TimerInitialized = 0;
	if( !s_TimerInitialized )
	{
		g_pTimer.reset( new CProfileTimer_Win32() );
		s_TimerInitialized = 1;
	}

	return (float)g_pTimer->GetExactTime();
}


void ProfileInit( void )
{
   unsigned int i;

   for( i=0; i<NUM_PROFILE_SAMPLES; i++ )
   {
      g_samples[i].bValid = false;
      g_history[i].bValid = false;

  }

   g_startProfile = GetExactTime();


   // reserve the buffer for texts that contains profile result
   g_vecstrProfileText.resize( NUM_PROFILE_SAMPLES + 2 );
   for( i=0; i<NUM_PROFILE_SAMPLES + 2; i++ )
   {
	   g_vecstrProfileText[i].reserve( 64 );
   }

}




void ProfileBegin( const char* name )
{
	if( !g_ProfileEnabled )
		return;

   unsigned int i = 0;

   while( i < NUM_PROFILE_SAMPLES && g_samples[i].bValid == true ) {
      if( strcmp( g_samples[i].szName, name ) == 0 ) {
         //Found the sample
         g_samples[i].iOpenProfiles++;
         g_samples[i].iProfileInstances++;
         g_samples[i].fStartTime = GetExactTime();
         assert( g_samples[i].iOpenProfiles == 1 ); //max 1 open at once
         return;
       }
       i++;	
   }

   if( i >= NUM_PROFILE_SAMPLES ) {
      assert( !"Exceeded Max Available Profile Samples" );
      return;
   }

   strcpy( g_samples[i].szName, name );
   g_samples[i].bValid = true;
   g_samples[i].iOpenProfiles = 1;
   g_samples[i].iProfileInstances = 1;
   g_samples[i].fAccumulator = 0.0f;
   g_samples[i].fStartTime = GetExactTime();
   g_samples[i].fChildrenSampleTime = 0.0f;
}


void ProfileEnd( const char* name )
{
	if( !g_ProfileEnabled )
		return;

   unsigned int i = 0;
   unsigned int numParents = 0;

   while( i < NUM_PROFILE_SAMPLES && g_samples[i].bValid == true )
   {
      if( strcmp( g_samples[i].szName, name ) == 0 )
      {  //Found the sample
         unsigned int inner = 0;
         int parent = -1;
         float fEndTime = GetExactTime();

         g_samples[i].iOpenProfiles--;

         //Count all parents and find the immediate parent
         while( g_samples[inner].bValid == true ) {
            if( g_samples[inner].iOpenProfiles > 0 )
            {  //Found a parent (any open profiles are parents)
               numParents++;
               if( parent < 0 )
               {  //Replace invalid parent (index)
                  parent = inner;
               }
               else if( g_samples[inner].fStartTime >=
                        g_samples[parent].fStartTime )
               {  //Replace with more immediate parent
                  parent = inner;
               }
            }
            inner++;
         }

         //Remember the current number of parents of the sample
         g_samples[i].iNumParents = numParents;

         if( parent >= 0 )
         {  //Record this time in fChildrenSampleTime (add it in)
            g_samples[parent].fChildrenSampleTime += fEndTime -
                                                     g_samples[i].fStartTime;
         }

         //Save sample time in accumulator
         g_samples[i].fAccumulator += fEndTime - g_samples[i].fStartTime;
         return;
      }
      i++;	
   }
}



void ProfileDumpOutputToBuffer( void )
{

	g_endProfile = GetExactTime();

	g_vecstrProfileText[0] = "  Ave :   Min :   Max :   # : Profile Name\n";
	g_vecstrProfileText[1] = "--------------------------------------------\n";

	size_t row = 0;
	unsigned int i;
	for( i = 0; i < NUM_PROFILE_SAMPLES && g_samples[i].bValid == true; i++ )
	{
		row = i + 2;
		unsigned int indent = 0;
		float sampleTime, percentTime, aveTime, minTime, maxTime;
		char line[256], name[256], indentedName[256];
		char ave[16], min[16], max[16], num[16];
			
		if( g_samples[i].iOpenProfiles < 0 )
		{
			assert( !"ProfileEnd() called without a ProfileBegin()" );	
		}
		else if( g_samples[i].iOpenProfiles > 0 )
		{
			assert( !"ProfileBegin() called without a ProfileEnd()" );
		}

		sampleTime = g_samples[i].fAccumulator - g_samples[i].fChildrenSampleTime;
		percentTime = ( sampleTime / (g_endProfile - g_startProfile ) ) * 100.0f;

		aveTime = minTime = maxTime = percentTime;

		//Add new measurement into the history and get the ave, min, and max
		StoreProfileInHistory( g_samples[i].szName, percentTime );
		GetProfileFromHistory( g_samples[i].szName, &aveTime, &minTime, &maxTime );

		//Format the data
		sprintf( ave, "%3.1f", aveTime );
		sprintf( min, "%3.1f", minTime );
		sprintf( max, "%3.1f", maxTime );
		sprintf( num, "%3d", g_samples[i].iProfileInstances );

		strcpy( indentedName, g_samples[i].szName );
		for( indent=0; indent<g_samples[i].iNumParents; indent++ )
		{
			sprintf( name, "   %s", indentedName );
			strcpy( indentedName, name );
		}

		sprintf(line,"%5s : %5s : %5s : %3s : %s\n", ave, min, max, num, indentedName);

		g_vecstrProfileText[ i + 2 ] = line;
	}

	// erase redundant text which was creted during the previous frame and still in the text buffer
	row++;
	size_t num_buffer_rows = g_vecstrProfileText.size();
	for( ; row<num_buffer_rows; row++ )
	{
		g_vecstrProfileText[row] = "";
	}

   {  //Reset samples for next frame
      unsigned int i;
      for( i=0; i<NUM_PROFILE_SAMPLES; i++ )
	  {
         g_samples[i].bValid = false;
      }
      g_startProfile = GetExactTime();
   }

   // update the state
   // - avoid changing the state between BeginProfile() and EndProfile() calls
   g_ProfileEnabled = g_ProfileRequestedState;
}


void ProfileEnable( bool enable )
{
	g_ProfileRequestedState = enable;
}


void StoreProfileInHistory( char* name, float percent )
{
   unsigned int i = 0;
   float oldRatio;
//   float newRatio = 0.8f * GetElapsedTime();

   float newRatio = 0.8f * (float)g_pTimer->GetFrameTime();

   if( newRatio > 1.0f )
   {
      newRatio = 1.0f;
   }

   oldRatio = 1.0f - newRatio;

   while( i < NUM_PROFILE_SAMPLES && g_history[i].bValid == true )
   {
      if( strcmp( g_history[i].szName, name ) == 0 )
      {  //Found the sample
         g_history[i].fAve = (g_history[i].fAve*oldRatio) + (percent*newRatio);
         if( percent < g_history[i].fMin ) {
            g_history[i].fMin = percent;
         }
         else {
            g_history[i].fMin = (g_history[i].fMin*oldRatio) + (percent*newRatio);
         }

         if( g_history[i].fMin < 0.0f ) {
            g_history[i].fMin = 0.0f;
		 }


         if( percent > g_history[i].fMax ) {
            g_history[i].fMax = percent;
         }
         else {
            g_history[i].fMax = (g_history[i].fMax*oldRatio) + (percent*newRatio);
         }
         return;
      }
      i++;
   }

   if( i < NUM_PROFILE_SAMPLES )
   {  //Add to history
      strcpy( g_history[i].szName, name );
      g_history[i].bValid = true;
      g_history[i].fAve = g_history[i].fMin = g_history[i].fMax = percent;
   }
   else {
      assert( !"Exceeded Max Available Profile Samples!");
   }
}


void GetProfileFromHistory( char* name, float* ave, float* min, float* max )
{
   unsigned int i = 0;
   while( i < NUM_PROFILE_SAMPLES && g_history[i].bValid == true )
   {
      if( strcmp( g_history[i].szName, name ) == 0 )
	  {  //Found the sample
         *ave = g_history[i].fAve;
         *min = g_history[i].fMin;
         *max = g_history[i].fMax;
         return;
	  }
      i++;
   }	
   *ave = *min = *max = 0.0f;
}

} // namespace amorphous
