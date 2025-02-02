#include <iostream>

struct DiceRoll
{
    int numDice;
    int diceSize;
    int bonus;
};

int diceRoll(DiceRoll roll)
{
    int total = 0;
    for (int i = 0; i < roll.numDice; i++)
    {
        total += (rand() % roll.diceSize) + 1;
    }
    total += roll.bonus;
    return total;
}

int statMod(int stat)
{
    return (stat - 10) / 2;
}

struct Weapon
{
    std::string name;
    int attackBonus;
    DiceRoll damage;
    int critRange;
    int critMultiplier;
};

std::ostream &operator<<(std::ostream &os, const Weapon &weapon)
{
    os << weapon.name << " | Attack Bonus: " << weapon.attackBonus
       << " | Damage: " << weapon.damage.numDice << "d" << weapon.damage.diceSize
       << "+" << weapon.damage.bonus << " | Crit: " << weapon.critRange
       << "x" << weapon.critMultiplier;
    return os;
}

struct PowerAttackResult
{
    int attackPenalty;
    int damageBonusOneHanded;
    int damageBonusTwoHanded;
};

PowerAttackResult calculatePowerAttack(bool powerAttacking, int bab)
{
    if (!powerAttacking)
    {
        return PowerAttackResult{0, 0, 0};
    }
    if (bab < 1)
    {
        throw std::invalid_argument("BAB must be at least 1 for Power Attack.");
    }

    PowerAttackResult result;

    // Correct attack penalty calculation (negative value)
    result.attackPenalty = -1 * (1 + (bab / 4));

    // Correct damage bonuses
    result.damageBonusOneHanded = -2 * result.attackPenalty;
    result.damageBonusTwoHanded = -3 * result.attackPenalty; // Two-handed gets 1.5x bonus

    return result;
}

std::ostream &operator<<(std::ostream &os, const PowerAttackResult &powerAttack)
{
    os << "Attack Penalty: " << powerAttack.attackPenalty
       << " | Damage Bonus (1H): " << powerAttack.damageBonusOneHanded
       << " | Damage Bonus (2H): " << powerAttack.damageBonusTwoHanded;
    return os;
}

int attackWithWeapon(int attackBonus, Weapon weapon, int armorClass)
{
    int damage = 0;
    DiceRoll attackRoll = {1, 20, 0};

    int roll = diceRoll(attackRoll);
    int attack = roll + attackBonus;

    return damage;
}

int main()
{
    // Character stats
    int str = 19;
    int bab = 6;
    int weaponFocus = 1;

    std::cout << "BAB: " << bab << std::endl;

    for (bool powerAttacking : {false, true})
    {
        PowerAttackResult powerAttack = calculatePowerAttack(powerAttacking, bab);

        std::cout << "Power Attacking: " << powerAttacking << " | " << powerAttack
                  << std::endl;

        for (bool twoWeapon : {false, true})
        {

            // Weapon stats
            Weapon primary = {"+1 Rapier", statMod(str) + 1, {1, 6, statMod(str) + powerAttack.damageBonusOneHanded + 1}, 18, 2};
            std::optional<Weapon> secondary;

            std::cout << "Primary: " << primary << std::endl;
            if (twoWeapon)
            {
                secondary.emplace(Weapon{"Shortsword, WF, off hand", statMod(str) + weaponFocus, {1, 6, (statMod(str) + powerAttack.damageBonusOneHanded) / 2}, 19, 2});
                std::cout
                    << "Secondary:  " << secondary.value() << std::endl;
            }
            std::cout << std::endl;

            for (int armorClass = 10; armorClass <= 30; armorClass++)
            {
                // TODO 1000 rounds of combat
                for (int round = 0; round < 1; round++)
                {
                    int damage = 0;

                    for (int effectiveBab = bab; effectiveBab >= 1; effectiveBab -= 5)
                    {

                        int attackBonus = effectiveBab + (twoWeapon ? -2 : 0) // Both weapons are light
                                          + powerAttack.attackPenalty;

                        std::cout << "Effective BAB: " << effectiveBab << " | Attack Bonus: " << attackBonus << std::endl;

                        // Primary attack
                        damage += attackWithWeapon(attackBonus, primary, armorClass);
                        if (secondary.has_value())
                        {
                            damage += attackWithWeapon(attackBonus, secondary.value(), armorClass);
                        }
                    }
                }
            }
        }
    }
    return 0;
}