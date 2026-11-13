//! The look and the lock of a work site's hold (docs/DESIGN.md, "Work site"): while the bar runs,
//! the one holding it works with their hands and can't walk, as with vanilla's building.
//!
//! All of it is on the holder's own machine, with the calls vanilla's own hold actions use:
//! - the lock is the character controller's movement switch (vanilla's item placement and the
//!   tutorial use it), set for the hold and always taken off again;
//! - digging with an entrenching tool in hand is vanilla's building animation
//!   (SCR_CampaignBuildingBuildUserAction: an item use with CMD_Item_Action, argument 1, the tool);
//!   a tool carried but put away is taken in hand first;
//! - any other work is done with the hands, with nothing in them: crouched when the thing is low (the
//!   stance vanilla's own crouch key sets), reaching down and taking hold again and again (the
//!   pick-up gesture vanilla plays when you take something from the ground, which needs no item),
//!   played again every 1.6 s while the bar runs. Only if the engine won't play that gesture, the
//!   item use vanilla plays at a mine being disarmed (SCR_DisarmMineAction: CMD_Item_Action,
//!   argument 0, on the thing itself). The first build tried only that last one, and with no tool
//!   nothing played: the site's box isn't an inventory item, which a mine is, so the engine
//!   refuses it. Whatever plays, the lock still holds.
//!
//! The server decides nothing here and is asked nothing: it still counts the work its own way
//! (RZ_WorkSites.WatchWorkers). A player is never left locked: the hold's end, letting go, a hit,
//! death or unconsciousness, another body, the site going, three quarters of a second with no bar and a time
//! cap a little over the hold's own time all release it (Watch, on the HUD's tick).

//------------------------------------------------------------------------------------------------
class RZ_WorkHold
{
	// What a site's hold looks like (RZ_WorkSiteComponent.GetLook, set by the server)
	static const int LOOK_LOCK = 1;		// can't walk while holding
	static const int LOOK_HANDS = 2;	// work with the hands at the thing
	static const int LOOK_DIG = 4;		// with an entrenching tool carried: vanilla's digging

	protected static const float CAP_EXTRA = 3;		// released this long past the hold's own time, whatever happens (s)
	protected static const float IDLE = 0.75;		// no bar for this long: nobody is holding (s)
	protected static const float HIT_PAUSE = 1.5;	// after a hit the hands are free this long before the work goes on (s)
	protected static const float TOOL_WAIT = 2.5;	// a tool taken out of the pack has this long to reach the hand (s)
	protected static const string ITEM_COMMAND = "CMD_Item_Action";
	protected static const int ARG_DIG = 1;			// vanilla's building: SCR_CampaignBuildingBuildUserAction
	protected static const int ARG_HANDS = 0;		// vanilla's work at a thing on the ground: SCR_DisarmMineAction
	protected static const float GESTURE_EVERY = 1.6;	// the hands' reach-and-take is played again this often (s)
	protected static const float LOW = 0.9;			// a thing lower than this under the holder's feet + this is worked at crouched (m)

	protected static bool s_bOn;
	protected static IEntity s_User;
	protected static IEntity s_Site;
	protected static BaseUserAction s_Action;
	protected static int s_iLook;
	protected static float s_fSeconds;
	protected static float s_fUntil;
	protected static float s_fSeen;
	protected static float s_fPauseUntil;
	protected static float s_fToolUntil;
	protected static bool s_bLocked;
	protected static bool s_bAnim;
	protected static bool s_bWantTool;
	protected static bool s_bGestures;		// the hands: the pick-up gesture, played again on the bar's frames
	protected static float s_fNextGesture;
	protected static bool s_bCrouched;		// crouched for the work: stood up again at the end

	//------------------------------------------------------------------------------------------------
	static bool IsOn()
	{
		return s_bOn;
	}

	//------------------------------------------------------------------------------------------------
	//! Just hit: the hands are free for a moment, and the bar waits.
	static bool Paused()
	{
		return RZ_UIW.Now() < s_fPauseUntil;
	}

	//------------------------------------------------------------------------------------------------
	//! The hold began (or goes on after a hit): lock and animate. Only for the local player's body.
	static void Begin(BaseUserAction action, IEntity user, IEntity site, float seconds, int look)
	{
		if (!user || user != SCR_PlayerController.GetLocalControlledEntity())
			return;

		if (s_bOn)
			End();

		// Nothing to show and nothing to lock (all three settings off): the hold is as it was
		if (look == 0)
			return;

		s_bOn = true;
		s_Action = action;
		s_User = user;
		s_Site = site;
		s_iLook = look;
		s_fSeconds = Math.Max(1, seconds);
		float now = RZ_UIW.Now();
		s_fSeen = now;
		s_fUntil = now + s_fSeconds + CAP_EXTRA;
		s_bWantTool = false;
		if ((look & LOOK_LOCK) != 0)
		{
			Lock(user, true);
			s_bLocked = true;
		}

		Animate();
	}

	//------------------------------------------------------------------------------------------------
	//! A frame of the bar: the hold is alive. A tool on its way to the hand starts the digging when
	//! it's there.
	static void Seen(BaseUserAction action)
	{
		if (!s_bOn || action != s_Action)
			return;

		float now = RZ_UIW.Now();
		s_fSeen = now;
		if (s_bGestures && now >= s_fNextGesture)
		{
			s_fNextGesture = now + GESTURE_EVERY;
			Gesture(s_User);
		}

		if (!s_bWantTool || s_bAnim)
			return;

		IEntity tool = HeldTool(s_User);
		if (tool)
		{
			s_bWantTool = false;
			s_bAnim = StartDig(s_User, tool, s_fSeconds);
			return;
		}

		// It never came out: work with the hands instead
		if (now >= s_fToolUntil)
		{
			s_bWantTool = false;
			if ((s_iLook & LOOK_HANDS) != 0)
				Hands();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The hold of this action is over, however it ended.
	static void EndFor(BaseUserAction action)
	{
		if (s_bOn && action == s_Action)
			End();
	}

	//------------------------------------------------------------------------------------------------
	//! Hit while holding: everything is let go at once, and the bar waits a moment.
	static void Hit(BaseUserAction action)
	{
		if (!s_bOn || action != s_Action)
			return;

		End();
		s_fPauseUntil = RZ_UIW.Now() + HIT_PAUSE;
	}

	//------------------------------------------------------------------------------------------------
	//! Release: the animation stopped, the legs free.
	static void End()
	{
		if (s_bAnim && s_User)
			StopAnim(s_User);

		if (s_bCrouched && s_User)
			Stand(s_User);

		if (s_bLocked)
		{
			if (s_User)
				Lock(s_User, false);

			// Another body since (a respawn, a Game Master's possession): that one is never left locked
			IEntity now = SCR_PlayerController.GetLocalControlledEntity();
			if (now && now != s_User)
				Lock(now, false);
		}

		s_bOn = false;
		s_bLocked = false;
		s_bAnim = false;
		s_bWantTool = false;
		s_bGestures = false;
		s_bCrouched = false;
		s_Action = null;
		s_User = null;
		s_Site = null;
	}

	//------------------------------------------------------------------------------------------------
	//! The safety net, on the HUD's tick: no hold may outlive its bar.
	static void Watch()
	{
		if (!s_bOn)
			return;

		float now = RZ_UIW.Now();
		IEntity me = SCR_PlayerController.GetLocalControlledEntity();
		if (!s_User || !s_Site || !s_Action || me != s_User || now >= s_fUntil || now - s_fSeen > IDLE || Down(s_User))
			End();
	}

	//------------------------------------------------------------------------------------------------
	//! Digging with the tool where the site digs and one is carried, else the hands.
	protected static void Animate()
	{
		s_bAnim = false;
		if ((s_iLook & LOOK_DIG) != 0)
		{
			IEntity held = HeldTool(s_User);
			if (held)
			{
				s_bAnim = StartDig(s_User, held, s_fSeconds);
				return;
			}

			if (DrawTool(s_User))
			{
				s_bWantTool = true;
				s_fToolUntil = RZ_UIW.Now() + TOOL_WAIT;
				return;
			}
		}

		if ((s_iLook & LOOK_HANDS) != 0)
			Hands();
	}

	//------------------------------------------------------------------------------------------------
	//! Work with the hands: crouched at a low thing, the reach-and-take played over and over; if the
	//! engine won't play it, the mine's item use on the thing.
	protected static void Hands()
	{
		if (s_Site && s_User && s_Site.GetOrigin()[1] < s_User.GetOrigin()[1] + LOW)
			s_bCrouched = Crouch(s_User);

		if (Gesture(s_User))
		{
			s_bGestures = true;
			s_fNextGesture = RZ_UIW.Now() + GESTURE_EVERY;
			return;
		}

		s_bAnim = StartHands(s_User, s_Site, s_fSeconds);
	}

	// ================================================================ Engine calls
	//------------------------------------------------------------------------------------------------
	protected static CharacterControllerComponent Controller(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return null;

		return character.GetCharacterController();
	}

	//------------------------------------------------------------------------------------------------
	//! Dead or out cold: nothing to hold on to.
	protected static bool Down(IEntity user)
	{
		CharacterControllerComponent controller = Controller(user);
		return !controller || controller.IsDead() || controller.IsUnconscious();
	}

	//------------------------------------------------------------------------------------------------
	//! The legs, on or off. VERIFY: CharacterControllerComponent.SetDisableMovementControls(true) on
	//! the player's own machine stops walking and turning the body while the hold key stays down and
	//! the camera still looks about (vanilla sets it for item placement); false gives them back.
	//! Wrong, and either you can still walk off mid-hold (the bar then starts again past 1.5 m), or
	//! the look locks too: turn Shared > Work sites > Lock movement off.
	protected static void Lock(IEntity user, bool on)
	{
		CharacterControllerComponent controller = Controller(user);
		if (controller)
			controller.SetDisableMovementControls(on);
	}

	//------------------------------------------------------------------------------------------------
	//! The entrenching tool in the hand (a gadget of vanilla's building-tool type), or null.
	protected static IEntity HeldTool(IEntity user)
	{
		SCR_GadgetManagerComponent gadgets = SCR_GadgetManagerComponent.GetGadgetManager(user);
		if (!gadgets)
			return null;

		SCR_GadgetComponent held = gadgets.GetHeldGadgetComponent();
		if (!held || held.GetType() != EGadgetType.BUILDING_TOOL || held.GetMode() != EGadgetMode.IN_HAND)
			return null;

		return gadgets.GetHeldGadget();
	}

	//------------------------------------------------------------------------------------------------
	//! A carried entrenching tool is taken in hand. True if there is one to take. VERIFY:
	//! SetGadgetMode(tool, IN_HAND) on the player's own machine draws a tool from the pack or a slot
	//! while a hold is running, and it's in hand within a couple of seconds. Wrong, and the digging
	//! falls back to the hands animation after 2.5 s.
	protected static bool DrawTool(IEntity user)
	{
		SCR_GadgetManagerComponent gadgets = SCR_GadgetManagerComponent.GetGadgetManager(user);
		if (!gadgets)
			return false;

		IEntity tool = gadgets.GetGadgetByType(EGadgetType.BUILDING_TOOL);
		if (!tool)
			return false;

		gadgets.SetGadgetMode(tool, EGadgetMode.IN_HAND);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Vanilla's building with the entrenching tool, for the hold's time. VERIFY: the item use with
	//! CMD_Item_Action and argument 1 on the held tool loops the digging as it does at a vanilla
	//! build site, other players see it (an item use replicates), and FinishItemUse ends it. Wrong,
	//! and the tool is held still: Shared > Work sites > Dig with the tool off uses the hands.
	protected static bool StartDig(IEntity user, IEntity tool, float seconds)
	{
		return UseItem(user, tool, ARG_DIG, seconds, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Down on one knee for the work, if standing. True if it crouched. VERIFY: SetStanceChange
	//! (STANCECHANGE_TOCROUCH) on the player's own machine crouches them as the crouch key does,
	//! others see it, and the movement lock set after it doesn't stop it. Wrong: they stand to work.
	protected static bool Crouch(IEntity user)
	{
		CharacterControllerComponent controller = Controller(user);
		if (!controller || controller.GetStance() != ECharacterStance.STAND)
			return false;

		controller.SetStanceChange(ECharacterStanceChange.STANCECHANGE_TOCROUCH);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Up again after the work, if still crouched as the work left them.
	protected static void Stand(IEntity user)
	{
		CharacterControllerComponent controller = Controller(user);
		if (controller && !controller.IsDead() && controller.GetStance() == ECharacterStance.CROUCH)
			controller.SetStanceChange(ECharacterStanceChange.STANCECHANGE_TOERECTED);
	}

	//------------------------------------------------------------------------------------------------
	//! The hands reach down and take hold: vanilla's pick-up gesture, which it plays when you take
	//! something off the ground into the pack, with no item named. True if it played. VERIFY: played
	//! again every 1.6 s while the bar runs, TryPlayItemGesture(EItemGesturePickUp) on the player's own
	//! machine shows the reach for them and for others nearby (as a pick-up from the ground does), with
	//! empty hands or a weapon slung, and the movement lock doesn't stop it. Wrong (false): the mine's
	//! item use is tried; it playing but nobody else seeing it is the item gestures' own replication.
	protected static bool Gesture(IEntity user)
	{
		CharacterControllerComponent controller = Controller(user);
		if (!controller || controller.IsDead() || controller.IsUnconscious())
			return false;

		if (controller.IsPlayingItemGesture())
			return true;

		if (!controller.CanPlayItemGesture())
			return false;

		return controller.TryPlayItemGesture(EItemGesture.EItemGesturePickUp);
	}

	//------------------------------------------------------------------------------------------------
	//! Work with the hands at the thing, as vanilla does at a mine being disarmed. VERIFY: an item
	//! use with CMD_Item_Action and argument 0 on an entity that isn't a held item (here the site's
	//! invisible box, in vanilla the mine on the ground) plays the kneel-and-work loop; the engine
	//! may refuse an entity with no item component (it then returns false and nothing plays), or
	//! pull the character's hand to the box. Wrong either way: Shared > Work sites > Hands
	//! animation off.
	protected static bool StartHands(IEntity user, IEntity site, float seconds)
	{
		return UseItem(user, site, ARG_HANDS, seconds, false);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool UseItem(IEntity user, IEntity item, int arg, float seconds, bool held)
	{
		CharacterControllerComponent controller = Controller(user);
		if (!controller || !item || !controller.CanUseItem())
			return false;

		CharacterAnimationComponent anim = controller.GetAnimationComponent();
		if (!anim)
			return false;

		ItemUseParameters params = new ItemUseParameters();
		params.SetEntity(item);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(true);
		params.SetCommandID(anim.BindCommand(ITEM_COMMAND));
		params.SetCommandIntArg(arg);
		// The engine's own end for a loop nobody stopped: a little past the hold
		params.SetMaxAnimLength(seconds + CAP_EXTRA);
		if (!held)
		{
			params.SetIsMainUserOfTheItem(true);
			params.SetKeepGadgetVisible(false);
		}

		return controller.TryUseItemOverrideParams(params);
	}

	//------------------------------------------------------------------------------------------------
	//! Ends the item use at once, as vanilla's build action does when it's cancelled or confirmed.
	protected static void StopAnim(IEntity user)
	{
		CharacterControllerComponent controller = Controller(user);
		if (!controller)
			return;

		CharacterAnimationComponent anim = controller.GetAnimationComponent();
		if (!anim)
			return;

		CharacterCommandHandlerComponent handler = anim.GetCommandHandler();
		if (handler)
			handler.FinishItemUse(true);
	}
}
