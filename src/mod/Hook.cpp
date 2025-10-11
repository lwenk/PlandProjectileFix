#include "MyMod.h"

#include <ll/api/memory/Hook.h>
#include <mc/entity/components_json_legacy/ProjectileComponent.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/player/Player.h>

#include <pland/PLand.h>
#include <pland/land/LandRegistry.h>


namespace my_mod {
using namespace land;

inline bool PreCheckLandExistsAndPermission(SharedLand const& ptr, mce::UUID const& uuid = mce::UUID::EMPTY()) {
    if (!ptr ||                                                       // 无领地
        (PLand::getInstance().getLandRegistry()->isOperator(uuid)) || // 管理员
        (ptr->getPermType(uuid) != LandPermType::Guest)               // 主人/成员
    ) {
        return true;
    }
    return false;
}

LL_TYPE_INSTANCE_HOOK(
    ProjectileComponentOnHitHook,
    HookPriority::Normal,
    ProjectileComponent,
    &ProjectileComponent::onHit,
    void,
    ::Actor&           owner,
    ::HitResult const& res
) {
    auto* db = PLand::getInstance().getLandRegistry();

    if ((int)res.mType < 2) {
        if (owner.hasType(::ActorType::Trident) && res.mType != HitResultType::Entity) return origin(owner, res);
        auto player = owner.getPlayerOwner();
        if (!player) return origin(owner, res);

        auto land = db->getLandAt(res.mPos, player->getDimensionId());
        if (!land) return origin(owner, res);
        if (land->getPermTable().allowFishingRodAndHook && owner.hasType(::ActorType::FishingHook))
            return origin(owner, res);
        if (land->getPermTable().allowProjectileCreate) return origin(owner, res);
        if (!PreCheckLandExistsAndPermission(land, player->getUuid())) {
            owner.despawn();
            return;
        }
    }
    // if (true) return;
    return origin(owner, res);
}
void hook() { ProjectileComponentOnHitHook::hook(); }
} // namespace my_mod