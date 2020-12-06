//============================================================================
// Name        : Kinematics.hpp
// Author      : Produced in the WPI AIM Lab
// Description : This file is strictly for defining an Abstract Kinematics Class
//				 all children of this type must implement the following
//============================================================================

#ifndef KINEMATICS_HPP_
#define KINEMATICS_HPP_

struct tempFK_outputs {};
struct tempIK_outputs {};
class Kinematics {
public:
	//================ Constructor ================
	Kinematics() {};
	virtual ~Kinematics(){};

	// Children classes must implement their own versions of Forward and Inverse Kinematics
	tempFK_outputs ForwardKinematics(tempFK_outputs A) { return A;};
	tempIK_outputs InverseKinematics(tempIK_outputs A) { return A;};

};



#endif /* KINEMATICS_HPP_ */
