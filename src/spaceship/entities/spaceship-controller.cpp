#include "spaceship-controller.h"

#include <spaceship/entities/spaceship.h>

using namespace spaceship;

SpaceshipController::~SpaceshipController()
{
	unpossess();
}

void SpaceshipController::possess( SharedPtr<Spaceship> ship )
{
	auto previous_ship = get_ship();

	//  unpossess
	_suppress_event = true;
	unpossess();
	_suppress_event = false;

	//auto ptr = get_shared_from_this<SpaceshipController>();

	//  force unpossess previous controller
	if ( auto controller = ship->wk_controller.lock() )
	{
		controller->unpossess();
	}

	//  possess
	_possessed_ship = ship;
	ship->wk_controller = as<SpaceshipController>();
	on_possess();

	//  invoke event
	on_possess_changed.invoke( previous_ship, ship );
}

void SpaceshipController::unpossess()
{
	auto previous_ship = get_ship();
	if ( !previous_ship ) return;

	//  reset controller of owned ship
	if ( previous_ship->wk_controller.lock().get() == this )
	{
		previous_ship->wk_controller.reset();
	}

	//  reset pointer
	on_unpossess();
	_possessed_ship.reset();

	//  invoke event
	if ( !_suppress_event )
	{
		on_possess_changed.invoke( previous_ship, nullptr );
	}
}
