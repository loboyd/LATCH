#include "descriptor.h"

descriptor::descriptor()
{}

void descriptor::set_value(int position, unsigned long long int value)
{
    values.at(position) = value;
}

