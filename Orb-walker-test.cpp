// Main orbwalker function
void __stdcall OrbWalker() {
 
	// Base Address + LocalPlayer + Champions vector
	// Standard setup
	DWORD BaseAddress = (DWORD)GetModuleHandle(NULL);
	GameObject* LocalPlayer = (GameObject*)(*(DWORD*)(BaseAddress + oLocalPlayer));
	std::vector<GameObject*> Champions;
 
        // This will be our target!
	GameObject* target = new GameObject();
 
	// Defining the functions we care about
	typedef bool(__thiscall* fnIsAlive)(GameObject* obj);
	typedef float(__cdecl* fnGetAttackDelay)(GameObject* obj);
	typedef float(__cdecl* fnGetAttackCastDelay)(GameObject* obj);
	typedef void(__thiscall* fnPrintChat)(DWORD, const char*, int);
	typedef int(__thiscall* fnNewIssueOrder)(DWORD HudThisPtr, int state, int IsAttack, int IsAttackCommand, int x, int y, int AttackAndMove);
 
	// Function objects
	fnGetAttackDelay GetAttackDelay = (fnGetAttackDelay)(BaseAddress + fGetAttackDelay);
	fnGetAttackCastDelay GetAttackCastDelay = (fnGetAttackCastDelay)(BaseAddress + fGetAttackCastDelay);
	fnPrintChat PrintChat = (fnPrintChat)(BaseAddress + fPrintChat);
	fnNewIssueOrder NewIssueOrder = (fnNewIssueOrder)(BaseAddress + fNewIssueOrder);
	fnIsAlive IsAlive = (fnIsAlive)(BaseAddress + fIsAlive);
 
	// Testing that we are properly injected
	PrintChat(BaseAddress + oChatClient, "Orbwalker", 0xFFFFFF);
 
	// Orbwalking
	while (true) {
 
		// Little delay so your computer doesn't die
		if (GetAsyncKeyState(VK_SPACE) && GetTickCount64() - lastLoop > 3) {
 
			// Checking if chat is open! Don't want to try spacegliding while typing...
			bool isChatOpen = *(int*)(*(DWORD*)(BaseAddress + oChatClient) + oChatOpen);
 
			if (!isChatOpen) {
				// Make sure Champions list is empty
				Champions.clear();
 
				// Load Champions' positions
				int vectorSize = (DWORD)(*(DWORD*)(*(DWORD*)(BaseAddress + HeroList) + 0x8));
 
				for (int i = 0; i < vectorSize; i++) {
					// Pointer to champion from the HeroClient
					GameObject* heroObject = (GameObject*)(*(DWORD*)(*(DWORD*)(*(DWORD*)(BaseAddress + HeroList) + 0x4) + 0x4 * i));
 
					// Check if it's the closest target that's been found (within reason)
					if (IsAlive(heroObject) && heroObject->targetable && heroObject->visible && heroObject->team != LocalPlayer->team && heroObject->position.DistanceTo(LocalPlayer->position) < 900 && heroObject->position.DistanceTo(LocalPlayer->position) < target->position.DistanceTo(LocalPlayer->position))
						// If so, that's the one!
						target = heroObject;
 
					// Add the heroObject to the list of champions anyway, you can use it later for other things
					Champions.push_back(heroObject);
				}
 
				// First check is to make sure that a target actually exists
				// Second check is to make sure that you aren't attacking during your attack's "cooldown"
				if (target->position.x != 0 && GetTickCount64() >= lastAttack + GetAttackDelay(LocalPlayer) * 1000.f) {
 
					// Load WorldToScreen
					LoadW2S();
 
					// Attack closest champion
					Vector2 AttackPosition = WorldToScreen(target->position);
 
					// Send input to attack
					DWORD HUDInput = *(DWORD*)(*(DWORD*)(BaseAddress + oHudInstance) + 0x24);
					NewIssueOrder(HUDInput, 0, 0, true, AttackPosition.x, AttackPosition.y, 0);
					NewIssueOrder(HUDInput, 1, 0, true, AttackPosition.x, AttackPosition.y, 0);
 
					// Update timer so that you don't try attacking on cooldown
					lastAttack = GetTickCount64();
				}
 
				// First check is to make sure you aren't moving too often
				// Second check is to make sure you aren't moving in your windup
				else if (GetTickCount64() > lastMove + 30 && GetTickCount64() >= lastAttack + GetAttackCastDelay(LocalPlayer) * 1000.f + Ping + 10) {
					
					// Load WorldToScreen
					LoadW2S();
					
					// Click with your mouse's current position
					Vector2 MovePosition = WorldToScreen(GetMousePos());
 
					// Send an input to right click at mouse position
					DWORD HUDInput = *(DWORD*)(*(DWORD*)(BaseAddress + oHudInstance) + 0x24);
					NewIssueOrder(HUDInput, 0, 0, true, MovePosition.x, MovePosition.y, 0);
					NewIssueOrder(HUDInput, 1, 0, true, MovePosition.x, MovePosition.y, 0);
 
					// Update timer so that you don't spam inputs and DC
					lastMove = GetTickCount64();
				}
			}
			// Update lastLoop so you don't overwork the thread
			lastLoop = GetTickCount64();
		}
	}
}
