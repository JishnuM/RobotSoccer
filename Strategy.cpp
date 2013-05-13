// Strategy.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "Strategy.h"
#include <math.h>

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
void Jish_Defend1(Robot *robot, Environment *env);
void Jish_Defend2(Robot *robot, Environment *env);
void Jish_Attack(Robot *robot, Environment *env);
void Jish_Support(Robot *robot, Environment *env);
void DoCross(Robot *robot, Environment *env);
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
}

extern "C" STRATEGY_API void Destroy ( Environment *env )
{
	// free any user data created in Create ( Environment * )
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
			Jish_Support(&env->home[3],env);
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

			Jish_Support(&env->home[3],env);
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
	int desired_angle = 0, theta_e = 0, d_angle = 0, vl, vr, vc = 125;

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

double PredictGoalRight (Environment *env )
{
	double dx = env->predictedBall.pos.x - env->currentBall.pos.x;
	double dy = env->predictedBall.pos.y - env->currentBall.pos.y;
	double k = env->predictedBall.pos.y -(dy/dx)*(env->predictedBall.pos.x);
	double goalY = (dy/dx)*GRIGHT + k;
	return goalY;
}

double PredictGoalLeft (Environment *env )
{
	double dx = env->currentBall.pos.x - env->lastBall.pos.x;
	double dy = env->currentBall.pos.y - env->lastBall.pos.y;
	double k = env->currentBall.pos.y -(dy/dx)*(env->currentBall.pos.x);
	double goalY = (dy/dx)*GLEFT + k;
	return goalY;
}

//void Goalie1 ( Robot *robot, Environment *env )
//{
	//
	//double yCentre = (env->fieldBounds.top + env->fieldBounds.bottom)/2 - 11;
	////double yCentre = 42;

	////Left if yellow, Right if blue
	////Arbitrary Constants Are Sad
	//double xCentre = (env->fieldBounds.right + 13);
	//
	////double xCentre = 90.5;

	//Vector3D nowBall = env->currentBall.pos;
	//Vector3D futureBall = env->predictedBall.pos;
	//double yIdeal = 0;
//	double xIdeal = xCentre;
//
//	double ideal_pos = PredictGoalRight(env);
	//if (ideal_pos > GTOPY){ideal_pos = GTOPY - 1;}
	//if (ideal_pos < GBOTY){ideal_pos = GBOTY + 1;}
	//
	//Position(robot,xCentre,ideal_pos);
//}
void Goalie1 ( Robot *robot, Environment *env )
{
    double xCentre = (env->fieldBounds.left + 2);

	double ideal_pos = env->currentBall.pos.y;
	if (ideal_pos > GTOPY){ideal_pos = GTOPY;}
	else if (ideal_pos < GBOTY){ideal_pos = GBOTY;}
	Position(robot,xCentre,ideal_pos);
}
void Jish_Defend1 (Robot *robot, Environment *env)
{
	Vector3D ball_pos = env->currentBall.pos;
	OpponentRobot* target;
	double target_x = 100;
	for (int i = 0; i < 5; i++)
	{
		if (env->opponent[i].pos.x < target_x)
		{
			target = &env->opponent[i];
			target_x = env->opponent[i].pos.x;
		}
	}
	//MoonFollowOpponent(robot,target);
	//akshat
	double x, y;
	
	if (ball_pos.x>50)
	{
	x = (ball_pos.x/2) - 50;
	y = ball_pos.y+1;
	Position(robot, x, y);
	}
	else if ((robot->pos.x > ball_pos.x-3)&& ball_pos.x < 50)
		{
			x = ball_pos.x-3;
			if (ball_pos.y<42)
				y = ball_pos.y-2;

			if (ball_pos.y>=42)
				y = ball_pos.y+2;

			Position(robot, x, y);
		}
	if ((robot->pos.x <= ball_pos.x-3) && robot->pos.x < 50)
		{
			x = ball_pos.x;
			y = ball_pos.y;
			Position(robot, x, y);
		}
}

void Jish_Defend2 (Robot *robot, Environment *env)
{
	Vector3D ball_pos = env->currentBall.pos;
	OpponentRobot* buffer = nullptr;
	OpponentRobot* target = nullptr;
	double buffer_x = 100, target_x = 100;
	for (int i = 0; i < 5; i++)
	{
		if (env->opponent[i].pos.x < buffer_x)
		{
			target = buffer;
			target_x = buffer_x;
			buffer = &env->opponent[i];
			buffer_x = env->opponent[i].pos.x;
		}
		else if (env->opponent[i].pos.x < target_x)
		{
			target = &env->opponent[i];
			target_x = env->opponent[i].pos.x;
		}
	}
	
	//MoonFollowOpponent(robot, target);
	//by akshat
    
	double x, y;
	if (ball_pos.x>50)
	{
	x = (ball_pos.x/2) - 50;
	y = ball_pos.y-1;
	Position(robot, x, y);
	}
	if ((robot->pos.x > ball_pos.x-3) && ball_pos.x < 50)
		{
			x = ball_pos.x-3;
			if (ball_pos.y<42)
				y = ball_pos.y-2;

			if (ball_pos.y>=42)
				y = ball_pos.y+2;
			Position(robot, x, y);
		}
	if ((robot->pos.x <= ball_pos.x-3) && robot->pos.x < 50)
		{
			x = ball_pos.x;
			y = ball_pos.y;
			Position(robot, x, y);
		}
	}

void Jish_Support(Robot *robot, Environment *env)
{
    double ball_dist = Distance(robot->pos.x,robot->pos.y,env->currentBall.pos.x,env->currentBall.pos.y);
	double goal_dist = Distance(robot->pos.x,robot->pos.y,FRIGHTX,(FBOT + (1/2)*FTOP));
	if (ball_dist < 1 && goal_dist < 30)
	{
		Position(robot,FRIGHTX,(FBOT + (1/2)*FTOP));
	}
	else if (ball_dist < 1)
	{
		if (env->currentBall.pos.x < robot->pos.x){
			double x_suggested = env->predictedBall.pos.x-2;
			if (x_suggested < FLEFTX + (1/3)*FRIGHTX){x_suggested = FLEFTX + (1/3)*FRIGHTX;}
			Position(robot,x_suggested,env->predictedBall.pos.y);
		}
		else
		{
			Position(robot,GRIGHT,GBOTY);
		}
	}
	else
	{
		double dx = env->currentBall.pos.x - env->lastBall.pos.x;
		double dy = env->currentBall.pos.y - env->lastBall.pos.y;
		double m1 = (dy/dx);
		double m2 = -(dx/dy);
		double k1 = env->currentBall.pos.y -(m1)*(env->currentBall.pos.x);
		double k2 = robot->pos.y - (m2)*(robot->pos.x);
		double x_return = (k2 - k1)/(m1 - m2);
		double y_return = m2*(x_return) + k2;

		if (x_return < FLEFTX + (1/3)*FRIGHTX){x_return = (FLEFTX + (1/3)*FRIGHTX);}
	
		Position(robot,x_return,y_return);
	}
}

void Jish_Attack(Robot *robot,Environment *env)
{
	//Get Distances to Ball
	double dist = Distance(env->currentBall.pos.x,env->currentBall.pos.y,robot->pos.x,robot->pos.y);
	
	/*if (robot->pos.x > (FLEFTX + (2/3)*FRIGHTX)){
		Position(robot,(FLEFTX + (2/3)*FRIGHTX),env->currentBall.pos.y);
	}
	else{*/
		if (env->currentBall.pos.x < robot->pos.x)
		{
			if(env->currentBall.pos.y > 42)
			{
				Position(robot,env->currentBall.pos.x-3,env->currentBall.pos.y-2);
			}
			else
			{
				Position(robot,env->currentBall.pos.x-3,env->currentBall.pos.y+2);
			}
		}
		else
		{
			if (dist > .1)
			{
				Position(robot,env->currentBall.pos.x,env->currentBall.pos.y);
			}
			else
			{
				Position(robot,GRIGHT,FTOP + (1/2)*FBOT);
			}
		}
	}

