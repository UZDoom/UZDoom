/*
 * ODOOM OQUAKE key actors.
 * These are runtime actors so OQUAKE key thing IDs render and can be collected in ODOOM.
 */

class OQGoldKey : DoomKey
{
	Default
	{
		Inventory.PickupMessage "Picked up a Gold key";
		Inventory.Icon "OQKGI0";
		Tag "Gold Key";
	}
	States
	{
	Spawn:
		// Doom key sprites rely on offset metadata; emulate that here for PNG sprites.
		OQKG A 10 A_SpriteOffset(0, -18);
		OQKG B 10 bright A_SpriteOffset(0, -18);
		loop;
	}
}

class OQSilverKey : DoomKey
{
	Default
	{
		Inventory.PickupMessage "Picked up a Silver key";
		Inventory.Icon "OQKSI0";
		Tag "Silver Key";
	}
	States
	{
	Spawn:
		// Doom key sprites rely on offset metadata; emulate that here for PNG sprites.
		OQKS A 10 A_SpriteOffset(0, -18);
		OQKS B 10 bright A_SpriteOffset(0, -18);
		loop;
	}
}
