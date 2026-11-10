[EntityEditorProps(category: "ReforgerZ", description: "A sealed road checkpoint at the Perimeter. If any of these exist in the world, road detection is skipped.")]
class RZ_CheckpointMarkerEntityClass : GenericEntityClass
{
}

//! Optional. Place one where a road reaches the edge of the map, inside the warning
//! zone, to put a sealed checkpoint there (barricade on the marker, guards outside
//! the playable area behind it). Use these when road detection finds the wrong roads
//! or none. Placing any markers turns road detection off. See RZ_Perimeter.
class RZ_CheckpointMarkerEntity : GenericEntity
{
	protected static ref array<RZ_CheckpointMarkerEntity> s_aAll;

	//------------------------------------------------------------------------------------------------
	static array<RZ_CheckpointMarkerEntity> GetAll()
	{
		if (!s_aAll)
			s_aAll = new array<RZ_CheckpointMarkerEntity>();

		return s_aAll;
	}

	//------------------------------------------------------------------------------------------------
	void RZ_CheckpointMarkerEntity(IEntitySource src, IEntity parent)
	{
		GetAll().Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_CheckpointMarkerEntity()
	{
		if (s_aAll)
			s_aAll.RemoveItem(this);
	}
}
