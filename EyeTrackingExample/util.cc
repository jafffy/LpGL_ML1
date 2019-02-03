#include "util.h"

#include <iomanip>
#include <sstream>

std::string toString(const MLTransform& ml_transform)
{
	std::ostringstream out;
	out << ml_transform.position.x << ','
		<< ml_transform.position.y << ','
		<< ml_transform.position.z << ','

		<< ml_transform.rotation.x << ',' // TODO: Fix it to print the rotational axis and the angle
		<< ml_transform.rotation.y << ','
		<< ml_transform.rotation.z << ','
		<< ml_transform.rotation.w;

	return out.str();
}