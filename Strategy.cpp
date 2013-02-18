// Strategy.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Strategy.h"
#include <iostream>
#include <math.h>

using namespace std;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}



const double PI = 3.1415923;

char myMessage[200]; //big enough???

void PredictBall ( Environment *env );
void Goalie1 ( Robot *robot, Environment *env );
void NearBound2 ( Robot *robot, double vl, double vr, Environment *env );
void Attack2 ( Robot *robot, Environment *env );
void Defend ( Robot *robot, Environment *env, double low, double high );
void Jish_Defend1(Robot *robot, Environment *env);
void Jish_Defend2(Robot *robot, Environment *env);
void Jish_Attack(Robot *robot, Environment *env);
// by moon at 9/2/2002
void MoonAttack (Robot *robot, Environment *env );
// just for testing to check whether the &env->opponent works or not
void MoonFollowOpponent (  Robot *robot, OpponentRobot *opponent );


void Velocity ( Robot *robot, int vl, int vr );
void Angle ( Robot *robot, int desired_angle);
void Position( Robot *robot, double x, double y );

extern "C" STRATEGY_API void Create ( Environment *env )
{
	// allocate user data and assign to env->userData
	// eg. env->userData = ( void * ) new MyVariables ();
}

extern "C" STRATEGY_API void Destroy ( Environment *env )
{
	// free any user data created in Create ( Environment * )

	// eg. if ( env->userData != NULL ) delete ( MyVariables * ) env->userData;
}


extern "C" STRATEGY_API void Strategy ( Environment *env )
{

	// the below codes are just for demonstration purpose....don't take this seriously please.

	int testInt = 100;
	int k;

	switch (env->gameState)
	{
		case 0:
			// default

			//MoonFollowOpponent ( &env->home [1], &env->opponent [2] );
			//MoonFollowOpponent ( &env->home [2], &env->opponent [3] );
			//MoonFollowOpponent ( &env->home [3], &env->opponent [4] );
			Jish_Defend1(&env->home[1], env);
			Jish_Defend2(&env->home[2], env);
			Jish_Attack(&env->home[3], env);
			Jish_Attack(&env->home[4], env);
			Goalie1 ( &env->home [0], env );

			break;

		case FREE_BALL:

			// Follow opponent guy
			//MoonFollowOpponent ( &env->home [1], &env->opponent [2] );
			//MoonFollowOpponent ( &env->home [2], &env->opponent [3] );
			//MoonFollowOpponent ( &env->home [3], &env->opponent [4] );
			Jish_Defend1(&env->home[1], env);
			Jish_Defend2(&env->home[2], env);

			Jish_Attack(&env->home[3], env);
			Jish_Attack(&env->home[4], env);

			// Goal keeper
			Goalie1 ( &env->home [0], env );

			// by moon at 24/03/2002
			// below code will not work.... never try....
			//	env->home[0].pos.x = 50;
			//	env->home[0].pos.y = 0;
			//	env->home[0].rotation = 20.0;

			break;

		case PLACE_KICK:
			MoonAttack ( &env->home [2], env );
			break;			
		case PENALTY_KICK:
			switch (env->whosBall)
			{
			case ANYONES_BALL:
				MoonAttack ( &env->home [1], env );
				break;
			case BLUE_BALL:
				MoonAttack ( &env->home [4], env );
				break;
			case YELLOW_BALL:
				MoonAttack ( &env->home [0], env );
				break;
			}
			break;

		case FREE_KICK:

			FILE * debugfile; 
			debugfile = fopen("debugfile.txt","a"); 
			
			for (k=0;k<=4;k++) 
				fprintf(debugfile, "robot: %d x: %f y: %f z: %f \n",
					k, env->opponent[k].pos.x, env->opponent[k].pos.y, 
					env->opponent[k].pos.z); 
			
			fclose(debugfile); 

			MoonAttack ( &env->home [0], env );

			break;

		case GOAL_KICK:
			MoonAttack ( &env->home [0], env );
			break;
  }
}


double Distance(double x1, double y1, double x2, double y2)
{
	return sqrt(pow(x1-x2,2)+pow(y1-y2,2));
}

void MoonAttack ( Robot *robot, Environment *env )
{
	//Velocity (robot, 127, 127);
	//Angle (robot, 45);
	PredictBall ( env );
	Position(robot, env->predictedBall.pos.x, env->predictedBall.pos.y);
	// Position(robot, 0.0, 0.0);
}

void MoonFollowOpponent ( Robot *robot, OpponentRobot *opponent )
{
	Position(robot, opponent->pos.x, opponent->pos.y);
}

void Velocity ( Robot *robot, int vl, int vr )
{
	robot->velocityLeft = vl;
	robot->velocityRight = vr;
}

// robot soccer system p329
void Angle ( Robot *robot, int desired_angle)
{
	int theta_e, vl, vr;
	theta_e = desired_angle - (int)robot->rotation;
	
	while (theta_e > 180) theta_e -= 360;
	while (theta_e < -180) theta_e += 360;

	if (theta_e < -90) theta_e += 180;
	
	else if (theta_e > 90) theta_e -= 180;

	if (abs(theta_e) > 50) 
	{
		vl = (int)(-9./90.0 * (double) theta_e);
		vr = (int)(9./90.0 * (double)theta_e);
	}
	else if (abs(theta_e) > 20)
	{
		vl = (int)(-11.0/90.0 * (double)theta_e);
		vr = (int)(11.0/90.0 * (double)theta_e);
	}
	else
	{
		vl = (int)(-13.0/90.0 * (double)theta_e);
		vr = (int)(13.0/90.0 * (double)theta_e);
	}
	Velocity (robot, vl, vr);
}

void Position( Robot *robot, double x, double y )
{
	int desired_angle = 0, theta_e = 0, d_angle = 0, vl, vr, vc = 70;

	double dx, dy, d_e, Ka = 10.0/90.0;
	dx = x - robot->pos.x;
	dy = y - robot->pos.y;

	d_e = sqrt(dx * dx + dy * dy);
	if (dx == 0 && dy == 0)
		desired_angle = 90;
	else
		desired_angle = (int)(180. / PI * atan2((double)(dy), (double)(dx)));
	theta_e = desired_angle - (int)robot->rotation;
	
	while (theta_e > 180) theta_e -= 360;
	while (theta_e < -180) theta_e += 360;

	if (d_e > 100.) 
		Ka = 17. / 90.;
	else if (d_e > 50)
		Ka = 19. / 90.;
	else if (d_e > 30)
		Ka = 21. / 90.;
	else if (d_e > 20)
		Ka = 23. / 90.;
	else 
		Ka = 25. / 90.;
	
	if (theta_e > 95 || theta_e < -95)
	{
		theta_e += 180;
		
		if (theta_e > 180) 
			theta_e -= 360;
		if (theta_e > 80)
			theta_e = 80;
		if (theta_e < -80)
			theta_e = -80;
		if (d_e < 5.0 && abs(theta_e) < 40)
			Ka = 0.1;
		vr = (int)(-vc * (1.0 / (1.0 + exp(-3.0 * d_e)) - 0.3) + Ka * theta_e);
		vl = (int)(-vc * (1.0 / (1.0 + exp(-3.0 * d_e)) - 0.3) - Ka * theta_e);
	}
	
	else if (theta_e < 85 && theta_e > -85)
	{
		if (d_e < 5.0 && abs(theta_e) < 40)
			Ka = 0.1;
		vr = (int)( vc * (1.0 / (1.0 + exp(-3.0 * d_e)) - 0.3) + Ka * theta_e);
		vl = (int)( vc * (1.0 / (1.0 + exp(-3.0 * d_e)) - 0.3) - Ka * theta_e);
	}

	else
	{
		vr = (int)(+.17 * theta_e);
		vl = (int)(-.17 * theta_e);
	}

	Velocity(robot, vl, vr);
}


void PredictBall ( Environment *env )
{
	double dx = env->currentBall.pos.x - env->lastBall.pos.x;
	double dy = env->currentBall.pos.y - env->lastBall.pos.y;
	env->predictedBall.pos.x = env->currentBall.pos.x + dx;
	env->predictedBall.pos.y = env->currentBall.pos.y + dy;

}

double PredictGoal (Environment *env )
{
	double dx = env->currentBall.pos.x - env->lastBall.pos.x;
	double dy = env->currentBall.pos.y - env->lastBall.pos.y;
	double k = env->currentBall.pos.y -(dy/dx)*(env->currentBall.pos.x);
	double goalY = (dy/dx)*GRIGHT + k;
	return goalY;
}

void Goalie1 ( Robot *robot, Environment *env )
{
	double yCentre = (env->fieldBounds.top + env->fieldBounds.bottom)/2 - 11;
	//double yCentre = 42;

	//Left if yellow, Right if blue
	//Arbitrary Constants Are Sad
	double xCentre = (env->fieldBounds.right + 15.5);
	
	//double xCentre = 90.5;

	Vector3D nowBall = env->currentBall.pos;
	Vector3D futureBall = env->predictedBall.pos;
	double yIdeal = 0;
	double xIdeal = xCentre;

	double fieldLength = env->fieldBounds.left - env->fieldBounds.right;
	double fieldRatio = (nowBall.x - env->fieldBounds.right)/fieldLength;

	if (fieldRatio > 1/5)
	{
		//double width = env->fieldBounds.top - env->fieldBounds.bottom;
		if (nowBall.y - env->fieldBounds.bottom < (1/3)*(env->fieldBounds.top - env->fieldBounds.bottom))
		{
			//cout<<"Bottom third"<<endl;
			yIdeal = yCentre - 3;

		}
		else if (nowBall.y - env->fieldBounds.bottom < (2/3)*(env->fieldBounds.top - env->fieldBounds.bottom))
		{
			//cout<<"Middle third"<<endl;
			yIdeal = yCentre;
		}
		else
		{
			//cout<<"Top third"<<endl;
			yIdeal = yCentre + 3;
		}
		Position(robot,xCentre,yIdeal);
	}
	//In your quarter of the field
	else
	{
		/*
		if(nowBall.y < (1/3)*(env->fieldBounds.top))
		{
			if (Distance(robot->pos.x,robot->pos.y,xCentre,yCentre) > 0.5)
			{
				Position(robot,xCentre,yCentre);
			}
			else
			{
				robot->velocityLeft = 125;
				robot->velocityRight = -125;
			}
		}
		else if(nowBall.y < (2/3)*(env->fieldBounds.top) )
		{
			xIdeal = nowBall.x;
			yIdeal = nowBall.y;
			Position(robot,xIdeal,yIdeal);
		}
		else
		{
			if (Distance(robot->pos.x,robot->pos.y,xCentre,yCentre) > 0.5)
			{
				Position(robot,xCentre,yCentre);
			}
			else
			{
				robot->velocityLeft = 125;
				robot->velocityRight = -125;
			}
		}
		*/
		if (nowBall.y > robot->pos.y)
		{
			robot->velocityLeft = 125;
			robot->velocityRight = -125;
		}
		else if (nowBall.y < robot->pos.y)
		{
			robot->velocityLeft = -125;
			robot->velocityRight = 125;
		}
		else
		{
			Position(robot,futureBall.x,futureBall.y);
		}
	}

	//double dist_to_ideal = Distance(robot->pos.x,robot->pos.y,xIdeal,yIdeal);

	//if (dist_to_ideal > 0.1)
	//{
		//cout<<"Shift!"<
	//else
	//{
		//cout<<"Meh"<<endl;
		//Angle(robot,0);
		//robot->velocityLeft = 0;
		//robot->velocityRight = 0;
	//}
	//Position(robot,xIdeal,yIdeal);
	//Angle(robot,0);
}

void Jish_Defend1 (Robot *robot, Environment *env)
{
	Vector3D ball_pos = env->currentBall.pos;
	OpponentRobot* target;
	double target_x = 0;
	for (int i = 0; i < 5; i++)
	{
		if (env->opponent[i].pos.x > target_x)
		{
			target = &env->opponent[i];
			target_x = env->opponent[i].pos.x;
		}
	}
	MoonFollowOpponent(robot,target);
}

void Jish_Defend2 (Robot *robot, Environment *env)
{
	Vector3D ball_pos = env->currentBall.pos;
	OpponentRobot* buffer = nullptr;
	OpponentRobot* target = nullptr;
	double buffer_x = 0, target_x = 0;
	for (int i = 0; i < 5; i++)
	{
		if (env->opponent[i].pos.x > buffer_x)
		{
			target = buffer;
			target_x = buffer_x;
			buffer = &env->opponent[i];
			buffer_x = env->opponent[i].pos.x;
		}
		else if (env->opponent[i].pos.x > target_x)
		{
			target = &env->opponent[i];
			target_x = env->opponent[i].pos.x;
		}
	}
	MoonFollowOpponent(robot,target);
}

void Jish_Attack(Robot *robot, Environment *env)
{
	double x_val = robot->pos.x;
	double y_val = robot->pos.y;
	Vector3D nowBall = env->currentBall.pos;
	//BlueBall
	if (env->whosBall == BLUE_BALL)
	{
		//If You Have the ball
		if (Distance(x_val,y_val,nowBall.x,nowBall.y) < 0.2)
		{
			//If Near the Goal
			if (Distance(x_val,y_val,GLEFT,(GTOPY+GBOTY)/2) <= 5)
			{
				Position(robot,GLEFT,(GTOPY+GBOTY)/2);
			}
			else
			{
				//Left (Bottom) Half
				if (y_val - env->fieldBounds.bottom < (1/2)*(env->fieldBounds.top - env->fieldBounds.bottom))
				{
					Position(robot,env->goalBounds.left,env->fieldBounds.bottom);
				}
				//Right Half
				else
				{
					Position(robot,env->goalBounds.left,env->fieldBounds.top);
				}
			}
		}
		//If You Don't Have the Ball
		else
		{
			Position(robot,env->currentBall.pos.x,env->currentBall.pos.y);
		}
	}
	//Yellow
	else if (env->whosBall == YELLOW_BALL)
	{
		/*
		if (y_val - env->fieldBounds.bottom < (1/2)*(env->fieldBounds.top - env->fieldBounds.bottom))
		{
			Position(robot,(GLEFT+GRIGHT)/2,(GBOTY + (1/3)*GTOPY));
		}
		else
		{
			Position(robot,(GLEFT+GRIGHT)/2,(GBOTY + (2/3)*GTOPY));
		}
		*/
		Position(robot,env->currentBall.pos.x,env->currentBall.pos.y);
	}
	else
	{
		/*
		if (y_val - env->fieldBounds.bottom < (1/2)*(env->fieldBounds.top - env->fieldBounds.bottom))
		{
			Position(robot,(GLEFT+GRIGHT)/2,(GBOTY + (1/3)*GTOPY));
		}
		else
		{
			Position(robot,(GLEFT+GRIGHT)/2,(GBOTY + (2/3)*GTOPY));
		}
		*/
		Position(robot,env->currentBall.pos.x,env->currentBall.pos.y);
	}

}

void Attack2 ( Robot *robot, Environment *env )
{
	Vector3D t = env->currentBall.pos;
	double r = robot->rotation;
	if ( r < 0 ) r += 360;
	if ( r > 360 ) r -= 360;
	double vl = 0, vr = 0;

	//Error Correction Code
	//If the ball position is recorded as outside limits, it is made as per limits
	//Why 3/2.5 etc?
	if ( t.y > env->fieldBounds.top - 2.5 ) t.y = env->fieldBounds.top - 2.5;
	if ( t.y < env->fieldBounds.bottom + 2.5 ) t.y = env->fieldBounds.bottom + 2.5;
	if ( t.x > env->fieldBounds.right - 3 ) t.x = env->fieldBounds.right - 3;
	if ( t.x < env->fieldBounds.left + 3 ) t.x = env->fieldBounds.left + 3;

	//Difference between robot position and ball position
	double dx = robot->pos.x - t.x;
	double dy = robot->pos.y - t.y;

	//Dafaq
	//Does this even care about the goal?
	
	double dxAdjusted = dx;
	double angleToPoint = 0;


	if ( fabs ( robot->pos.y - t.y ) > 7 || t.x > robot->pos.x )
		dxAdjusted -= 5;

	if ( dxAdjusted == 0 )
	{
		if ( dy > 0 )
			angleToPoint = 270;
		else
			angleToPoint = 90;
	}
	else if ( dy == 0 )
	{
		if ( dxAdjusted > 0 )
			angleToPoint = 360;
		else
			angleToPoint = 180;
		
	}
	else
		angleToPoint = atan ( fabs ( dy / dx ) ) * 180.0 / PI;

	if ( dxAdjusted > 0 )
	{
		if ( dy > 0 )
			angleToPoint -= 180;
		else if ( dy < 0 )
			angleToPoint = 180 - angleToPoint;
	}
	if ( dxAdjusted < 0 )
	{
		if ( dy > 0 )
			angleToPoint = - angleToPoint;
		else if ( dy < 0 )
			angleToPoint = 90 - angleToPoint;
	}

	if ( angleToPoint < 0 ) angleToPoint = angleToPoint + 360;
	if ( angleToPoint > 360 ) angleToPoint = angleToPoint - 360;
	if ( angleToPoint > 360 ) angleToPoint = angleToPoint - 360;

	double c = r;

	double angleDiff = fabs ( r - angleToPoint );

	if ( angleDiff < 40 )
	{
		vl = 100;
		vr = 100;
		if ( c > angleToPoint )
			vl -= 10;
		if ( c < angleToPoint )
			vr -= 10;
	}
	else
	{
		if ( r > angleToPoint )
		{
			if ( angleDiff > 180 )
				vl += 360 - angleDiff;
			else
				vr += angleDiff;
		}
		if ( r < angleToPoint )
		{
			if ( angleDiff > 180 )
				vr += 360 - angleDiff;
			else
				vl += angleDiff;
		}
	}

	NearBound2 ( robot, vl, vr, env );
}

void NearBound2 ( Robot *robot, double vl, double vr, Environment *env )
{
	//Vector3D t = env->currentBall.pos;

	Vector3D a = robot->pos;
	double r = robot->rotation;

	if ( a.y > env->fieldBounds.top - 15 && r > 45 && r < 130 )
	{
		if ( vl > 0 )
			vl /= 3;
		if ( vr > 0 )
			vr /= 3;
	}

	if ( a.y < env->fieldBounds.bottom + 15 && r < -45 && r > -130 )
	{
		if ( vl > 0 ) vl /= 3;
		if ( vr > 0 ) vr /= 3;
	}

	if ( a.x > env->fieldBounds.right - 10 )
	{
		if ( vl > 0 )
			vl /= 2;
		if ( vr > 0 )
			vr /= 2;
	}

	if ( a.x < env->fieldBounds.left + 10 )
	{
		if ( vl > 0 )
			vl /= 2;
		if ( vr > 0 )
			vr /= 2;
	}

	robot->velocityLeft = vl;
	robot->velocityRight = vr;
}

void Defend ( Robot *robot, Environment *env, double low, double high )
{
	double vl = 0, vr = 0;
	Vector3D z = env->currentBall.pos;

	double Tx = env->goalBounds.right - z.x;
	double Ty = env->fieldBounds.top - z.y;
	Vector3D a = robot->pos;
	a.x = env->goalBounds.right - a.x;
	a.y = env->fieldBounds.top - a.y;

	if ( a.y > Ty + 0.9 && a.y > low )
	{
		vl = -100;
		vr = -100;
	}
	if ( a.y < Ty - 0.9 && a.y < high )
	{
		vl = 100;
		vr = 100;
	}
	if ( a.y < low )
	{
		vl = 100;
		vr = 100;
	}
	if ( a.y > high )
	{
		vl = -100;
		vr = -100;
	}

	double Tr = robot->rotation;

	if ( Tr < 0.001 )
		Tr += 360;
	if ( Tr > 360.001 )
		Tr -= 360;
	if ( Tr > 270.5 )
		vr += fabs ( Tr - 270 );
	else if ( Tr < 269.5 )
		vl += fabs ( Tr - 270 );

	NearBound2 ( robot, vl ,vr, env );
}
/*
void JishnuAttack(Robot *robot, Environment *env)
{
	//2 Cases. Either ball is near or ball is not.
	Vector 3D currentPos = env->currentBall.pos;


}

void JishnuGoalie(Robot *robot, Environment *env)
{

}

void JishnuDefend(Robot *robot, Environment *env)
{

}
*/

