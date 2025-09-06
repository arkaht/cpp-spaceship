#include "spaceship-controller.h"

#include <spaceship/entities/spaceship.h>

using namespace spaceship;

SpaceshipController::~SpaceshipController()
{
	unpossess();
}

void SpaceshipController::possess( const SharedPtr<Spaceship>& ship )
{
	const SharedPtr<Spaceship> previous_ship = get_ship();

	// Un-possess
	_suppress_event = true;
	unpossess();
	_suppress_event = false;

	// Force un-possess previous controller
	if ( const SharedPtr<SpaceshipController> controller = ship->wk_controller.lock())
	{
		controller->unpossess();
	}

	// Possess
	_possessed_ship = ship;
	ship->wk_controller = as<SpaceshipController>();
	on_possess();

	// Invoke event
	on_possess_changed.invoke( previous_ship, ship );
}

void SpaceshipController::unpossess()
{
	const SharedPtr<Spaceship> previous_ship = get_ship();
	if ( !previous_ship ) return;

	// Reset controller of owned ship
	if ( previous_ship->wk_controller.lock().get() == this )
	{
		previous_ship->wk_controller.reset();
	}

	// Reset pointer
	on_unpossess();
	_possessed_ship.reset();

	// Invoke event
	if ( !_suppress_event )
	{
		on_possess_changed.invoke( previous_ship, nullptr );
	}
}
