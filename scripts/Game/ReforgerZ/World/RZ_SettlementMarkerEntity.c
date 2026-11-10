[EntityEditorProps(category: "ReforgerZ", description: "Defines a settlement by hand. If any of these exist in the world, auto-detection is skipped.")]
class RZ_SettlementMarkerEntityClass : GenericEntityClass
{
}

//! Optional. Mission makers can drop these on a map to control exactly where
//! towns are. Without any markers, RZ_SettlementScanner finds towns itself.
class RZ_SettlementMarkerEntity : GenericEntity
{
	[Attribute("Settlement", desc: "Display name")]
	protected string m_sName;

	[Attribute("200", UIWidgets.Slider, "Radius in metres", "50 1000 10")]
	protected float m_fRadius;

	[Attribute("1", UIWidgets.Slider, "Size class: 1 village, 2 town, 3 city", "1 3 1")]
	protected int m_iSize;

	[Attribute("30", UIWidgets.Slider, "Starting infection", "0 100 1")]
	protected float m_fStartInfection;

	[Attribute("0", desc: "Spawn the cure lab in this settlement")]
	protected bool m_bLab;

	protected static ref array<RZ_SettlementMarkerEntity> s_aAll;

	//------------------------------------------------------------------------------------------------
	static array<RZ_SettlementMarkerEntity> GetAll()
	{
		if (!s_aAll)
			s_aAll = new array<RZ_SettlementMarkerEntity>();

		return s_aAll;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Settlement ToSettlement()
	{
		RZ_Settlement s = new RZ_Settlement();
		s.m_sName = m_sName;
		s.m_vCenter = GetOrigin();
		s.m_fRadius = m_fRadius;
		s.m_iSize = m_iSize;
		s.m_fInfection = m_fStartInfection;
		s.m_bHasLab = m_bLab;
		return s;
	}

	//------------------------------------------------------------------------------------------------
	void RZ_SettlementMarkerEntity(IEntitySource src, IEntity parent)
	{
		GetAll().Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_SettlementMarkerEntity()
	{
		if (s_aAll)
			s_aAll.RemoveItem(this);
	}
}
