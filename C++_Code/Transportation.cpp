#include "Transportation.h"

transportation::transportation()
{
	this->g = -1;
	this->h = -1;
	this->last = nullptr;
}

transportation::transportation(string stop_name, double g, double h, transportation * last, string represent_number)
{
	this->stop_name = stop_name;
	this->g = g;
	this->h = h;
	this->last = last;
	this->represent_number = represent_number;
}

transportation::~transportation()
{
}
