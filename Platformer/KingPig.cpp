#include "KingPig.h"

KingPig::KingPig(MONSTER_TYPE type, FPOINT pos, float speed)
	: Monster(type, pos, speed)
{
	isOnGround = false;
	canMoveLeft = false;
	canMoveRight = false;

	traceDistance = 200.f;
	state = MONSTER_STATE_NONE;

	currentImage = nullptr;

	ani_Idle = nullptr;
	ani_Move = nullptr;
	ani_Attack = nullptr;
	ani_Hit = nullptr;
	ani_Dead = nullptr;

	deadTime = 0.f;
	deadDelayTime = 1.f;

	maxHp = 5;
	hp = maxHp;
}

void KingPig::Init()
{
	imageAnimations[KING_PIG_IMAGE::IDLE].Load(TEXT("Data/Monster/King_Pig_Idle_212.bmp"));
	imageAnimations[KING_PIG_IMAGE::ATTACK].Load(TEXT("Data/Monster/King_Pig_Attack_25.bmp"));
	imageAnimations[KING_PIG_IMAGE::MOVE].Load(TEXT("Data/Monster/King_Pig_Move_26.bmp"));
	imageAnimations[KING_PIG_IMAGE::HIT].Load(TEXT("Data/Monster/King_Pig_Hit_22.bmp"));
	imageAnimations[KING_PIG_IMAGE::DEAD].Load(TEXT("Data/Monster/King_Pig_Dead_24.bmp"));

	DefineAnimation(ani_Idle, KING_PIG_IMAGE::IDLE, 12, 2, 10);
	DefineAnimation(ani_Move, KING_PIG_IMAGE::MOVE, 6, 2, 10);
	DefineAnimation(ani_Attack, KING_PIG_IMAGE::ATTACK, 5, 2, 10);
	DefineAnimation(ani_Hit, KING_PIG_IMAGE::HIT, 2, 2, 10);
	DefineAnimation(ani_Dead, KING_PIG_IMAGE::DEAD, 4, 2, 10);

	SetAnimation(MONSTER_STATE::LEFT_IDLE);
}

void KingPig::Update(float deltaTime)
{
	CheckMonsterState();
	CheckCollision();
	Move(deltaTime);

	switch (state)
	{
	case RIGHT_IDLE:
	case LEFT_IDLE:
		if (ani_Idle)
			ani_Idle->FrameUpdate(deltaTime);
		break;

	case RIGHT_MOVE:
	case LEFT_MOVE:
		if (ani_Move)
			ani_Move->FrameUpdate(deltaTime);
		break;

	case RIGHT_ATTACK:
	case LEFT_ATTACK:
		if (ani_Attack)
			ani_Attack->FrameUpdate(deltaTime);

		if (!ani_Attack->IsPlay())
			SetAnimation(stateBefore);
		break;

	case RIGHT_HIT:
	case LEFT_HIT:
		if (ani_Hit)
			ani_Hit->FrameUpdate(deltaTime);

		if (!ani_Hit->IsPlay())
			SetAnimation(stateBefore);
		break;

	case RIGHT_DEAD:
	case LEFT_DEAD:
		if (ani_Dead)
			ani_Dead->FrameUpdate(deltaTime);
		deadTime += deltaTime;

		if (deadTime > deadDelayTime)
		{
			isDead = true;
			deadTime = 0.f;
		}
		break;

	default:
		break;
	}
}

void KingPig::Render(HDC hdc)
{
	switch (state)
	{
	case RIGHT_IDLE:
	case LEFT_IDLE:
		if (currentImage)
			currentImage->DrawAniRender(hdc, pos.x, pos.y, ani_Idle);
		break;

	case RIGHT_MOVE:
	case LEFT_MOVE:
		if (currentImage)
			currentImage->DrawAniRender(hdc, pos.x, pos.y, ani_Move);
		break;

	case RIGHT_ATTACK:
	case LEFT_ATTACK:
		if (currentImage)
			currentImage->DrawAniRender(hdc, pos.x, pos.y, ani_Attack);
		break;

	case RIGHT_HIT:
	case LEFT_HIT:
		if (currentImage)
			currentImage->DrawAniRender(hdc, pos.x, pos.y, ani_Hit);
		break;

	case RIGHT_DEAD:
	case LEFT_DEAD:
		if (currentImage)
			currentImage->DrawAniRender(hdc, pos.x, pos.y, ani_Dead);
		break;

	default:
		break;
	}

	SelectObject(hdc, GetStockObject(NULL_BRUSH));
	Rectangle(hdc, pos.x, pos.y, pos.x + size.width, pos.y + size.height);
}

void KingPig::Release()
{
	imageAnimations[KING_PIG_IMAGE::IDLE].Release();
	imageAnimations[KING_PIG_IMAGE::ATTACK].Release();
	imageAnimations[KING_PIG_IMAGE::MOVE].Release();
	imageAnimations[KING_PIG_IMAGE::HIT].Release();
	imageAnimations[KING_PIG_IMAGE::DEAD].Release();

	SAFE_DELETE(ani_Idle);
	SAFE_DELETE(ani_Move);
	SAFE_DELETE(ani_Attack);
	SAFE_DELETE(ani_Hit);
	SAFE_DELETE(ani_Dead);
}

void KingPig::Move(float deltaTime)
{
	if (state == MONSTER_STATE::LEFT_MOVE && canMoveLeft)
	{
		pos.x -= speed * deltaTime;
	}

	if (state == MONSTER_STATE::RIGHT_MOVE && canMoveRight)
	{
		pos.x += speed * deltaTime;
	}

	if (!isOnGround)
	{
		pos.y += gravity * deltaTime;
	}
}

void KingPig::SetAnimation(MONSTER_STATE _state)
{
	if (state == _state)
		return;

	ani_Idle->Stop();
	ani_Move->Stop();
	ani_Attack->Stop();
	ani_Hit->Stop();
	ani_Dead->Stop();

	stateBefore = state;
	state = _state;

	switch (state)
	{
	case RIGHT_IDLE:
		ani_Idle->SetPlayFrame(0, 11, true, true);
		ani_Idle->Start();
		currentImage = &imageAnimations[KING_PIG_IMAGE::IDLE];
		size.width = ani_Idle->GetFrameWidth();
		size.height = ani_Idle->GetFrameHeight();
		break;
	case LEFT_IDLE:
		ani_Idle->SetPlayFrame(12, 23, false, true);
		ani_Idle->Start();
		currentImage = &imageAnimations[KING_PIG_IMAGE::IDLE];
		size.width = ani_Idle->GetFrameWidth();
		size.height = ani_Idle->GetFrameHeight();
		break;
	case RIGHT_MOVE:
		ani_Move->SetPlayFrame(0, 5, true, true);
		ani_Move->Start();
		currentImage = &imageAnimations[KING_PIG_IMAGE::MOVE];
		size.width = ani_Move->GetFrameWidth();
		size.height = ani_Move->GetFrameHeight();
		break;
	case LEFT_MOVE:
		ani_Move->SetPlayFrame(6, 11, false, true);
		ani_Move->Start();
		currentImage = &imageAnimations[KING_PIG_IMAGE::MOVE];
		size.width = ani_Move->GetFrameWidth();
		size.height = ani_Move->GetFrameHeight();
		break;
	case RIGHT_ATTACK:
		ani_Attack->SetPlayFrame(0, 4, true, false);
		ani_Attack->Start();
		currentImage = &imageAnimations[KING_PIG_IMAGE::ATTACK];
		size.width = ani_Attack->GetFrameWidth();
		size.height = ani_Attack->GetFrameHeight();
		break;
	case LEFT_ATTACK:
		ani_Attack->SetPlayFrame(5, 9, false, false);
		ani_Attack->Start();
		currentImage = &imageAnimations[KING_PIG_IMAGE::ATTACK];
		size.width = ani_Attack->GetFrameWidth();
		size.height = ani_Attack->GetFrameHeight();
		break;
	case RIGHT_HIT:
		ani_Hit->SetPlayFrame(0, 1, true, false);
		ani_Hit->Start();
		currentImage = &imageAnimations[KING_PIG_IMAGE::HIT];
		size.width = ani_Hit->GetFrameWidth();
		size.height = ani_Hit->GetFrameHeight();
		break;
	case LEFT_HIT:
		ani_Hit->SetPlayFrame(2, 3, false, false);
		ani_Hit->Start();
		currentImage = &imageAnimations[KING_PIG_IMAGE::HIT];
		size.width = ani_Hit->GetFrameWidth();
		size.height = ani_Hit->GetFrameHeight();
		break;
	case RIGHT_DEAD:
		ani_Dead->SetPlayFrame(0, 3, true, false);
		ani_Dead->Start();
		currentImage = &imageAnimations[KING_PIG_IMAGE::DEAD];
		size.width = ani_Dead->GetFrameWidth();
		size.height = ani_Dead->GetFrameHeight();
		break;

	case LEFT_DEAD:
		ani_Dead->SetPlayFrame(4, 7, false, false);
		ani_Dead->Start();
		currentImage = &imageAnimations[KING_PIG_IMAGE::DEAD];
		size.width = ani_Dead->GetFrameWidth();
		size.height = ani_Dead->GetFrameHeight();
		break;
	default:
		break;
	}
}

void KingPig::CheckMonsterState()
{
	if (player == nullptr)return;
	if (IsAttacking() || IsHitting() || IsDead())return;

	// 추적 거리 안에 오면 다가간다
	if (IsCollision(GetRect(), player->GetRect()))
	{
		if (player->GetPos().x < pos.x)
		{
			SetAnimation(MONSTER_STATE::LEFT_ATTACK);
		}
		else
		{
			SetAnimation(MONSTER_STATE::RIGHT_ATTACK);
		}
	}
	else if (Distance(player->GetPos(), pos) <= traceDistance)
	{
		if (player->GetPos().x < pos.x)
		{
			SetAnimation(MONSTER_STATE::LEFT_MOVE);
		}
		else
		{
			SetAnimation(MONSTER_STATE::RIGHT_MOVE);
		}
	}
	// 아니라면 Idle
	else
	{
		if (IsLeft())
		{
			SetAnimation(MONSTER_STATE::LEFT_IDLE);
		}
		else
		{
			SetAnimation(MONSTER_STATE::RIGHT_IDLE);
		}
	}
}

void KingPig::CheckCollision()
{
	if (!player)return;

	// 플레이어를 공격
	if (!player->IsAttacked())
	{
		if (IsCollision(player->GetRect(), GetRect()))
		{
			int force = (IsLeft()) ? -20 : 20;
			player->AddForceX(force);
			player->SetIsAttacked(true);
		}
	}

	// 플레이어에 공격 당했을 때
	if (player->IsAttacking() && player->GetCanAttack())
	{
		if (IsCollision(player->GetAttackRect(), GetRect()))
		{
			player->SetCanAttack(false);
			hp--;

			if (hp <= 0)
			{
				if (IsLeft())
					SetAnimation(MONSTER_STATE::LEFT_DEAD);
				else
					SetAnimation(MONSTER_STATE::RIGHT_DEAD);
			}
			else
			{
				if (IsLeft())
					SetAnimation(MONSTER_STATE::LEFT_HIT);
				else
					SetAnimation(MONSTER_STATE::RIGHT_HIT);
			}
		}
	}
}

void KingPig::DefineAnimation(Animation*& animation, UINT state, int width, int height, int fps)
{
	animation = new Animation;
	int w = imageAnimations[state].GetBitmap().bmWidth;
	int h = imageAnimations[state].GetBitmap().bmHeight;

	animation->Init(w, h, w / width, h / height);
	animation->SetFPS(fps);
}