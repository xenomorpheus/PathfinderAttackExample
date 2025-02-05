#include <iostream>
#include <vector>
#include <format>

#include "logger.hpp"

namespace
{

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
           << ", Damage Bonus (1H): " << powerAttack.damageBonusOneHanded
           << ", Damage Bonus (2H): " << powerAttack.damageBonusTwoHanded;
        return os;
    }

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
        os << weapon.name << ", Attack Bonus: " << weapon.attackBonus
           << ", Damage: " << weapon.damage.numDice << "d" << weapon.damage.diceSize
           << "+" << weapon.damage.bonus << ", Crit: " << weapon.critRange
           << "x" << weapon.critMultiplier;
        return os;
    }

    int statMod(int stat)
    {
        return (stat - 10) / 2;
    }
    class PathfinderAttack
    {
    private:
        Logger &logger;

        int attackWithWeapon(int attackBonus, Weapon weapon, int armorClass)
        {
            int damage = 0;
            DiceRoll attackRoll = {1, 20, 0};

            int roll = diceRoll(attackRoll);
            if (roll == 1)
            {
                logger.log(LogLevel::TRACE, std::ostringstream() << "Fumbled with " << weapon.name);
                return 0;
            }
            int attack = roll + attackBonus + weapon.attackBonus;
            logger.log(LogLevel::TRACE, std::ostringstream() << "Attack Roll: " << attack << ", Dice Roll:" << roll << ", AC: " << armorClass);
            if (roll >= weapon.critRange)
            {
                int confirmRoll = diceRoll(attackRoll);
                int confirmAttack = confirmRoll + attackBonus + weapon.attackBonus;
                if (confirmAttack < armorClass)
                {
                    logger.log(LogLevel::TRACE, std::ostringstream() << "Hit with " << weapon.name);
                    damage += diceRoll(weapon.damage);
                }
                else
                {
                    logger.log(LogLevel::TRACE, std::ostringstream() << "Confirmed critical hit with " << weapon.name);
                    damage += diceRoll(weapon.damage) * weapon.critMultiplier;
                }
            }
            else if (attack >= armorClass)
            {
                logger.log(LogLevel::TRACE, std::ostringstream() << "Hit with " << weapon.name);
                damage += diceRoll(weapon.damage);
            }
            else
            {
                logger.log(LogLevel::TRACE, std::ostringstream() << "Missed with " << weapon.name);
            }

            return damage;
        }

        int attackSequenceDamage(const std::vector<Weapon> &weapons, int bab, const PowerAttackResult &powerAttack, int armorClass)
        {
            if (weapons.empty())
            {
                logger.log(LogLevel::ERROR, std::ostringstream() << "No weapons to attack with!");
                return 0;
            }
            if (weapons.size() > 2)
            {
                logger.log(LogLevel::ERROR, std::ostringstream() << "Too many weapons to attack with!");
                return 0;
            }
            int damageTotal = 0;
            for (int effectiveBab = bab; effectiveBab >= 1; effectiveBab -= 5)
            {
                int attackBonus = effectiveBab + (weapons.size() == 2 ? -2 : 0) // Assume both weapons are light
                                  + powerAttack.attackPenalty;
                logger.log(LogLevel::TRACE, std::ostringstream() << "Effective BAB: " << effectiveBab << ", Attack Bonus: " << attackBonus);

                for (const Weapon &weapon : weapons)
                {
                    damageTotal += attackWithWeapon(attackBonus, weapon, armorClass);
                }
            }
            logger.log(LogLevel::DEBUG, std::ostringstream() << "Attack sequence damage: " << damageTotal);
            return damageTotal;
        }

    public:
        PathfinderAttack(Logger &logger) : logger{logger} {}
        ~PathfinderAttack() {}

        double averageDamage(const std::vector<Weapon> &weapons, int bab, const PowerAttackResult &powerAttack, int armorClass, int sequences = 1'000)
        {
            long damage = 0;
            for (int sequence = 0; sequence < sequences; sequence++)
            {
                damage += attackSequenceDamage(weapons, bab, powerAttack, armorClass);
            }
            return (double)damage / sequences;
        }
    };

}

void showAverageDamagePerAc()
{
    // Character stats
    int str = 19;
    int bab = 6;
    int weaponFocus = 1;

    Logger logger("pathfinder_attack.log");
    logger.setLogLevel(LogLevel::INFO);
    PathfinderAttack pathfinderAttack(logger);

    logger.log(LogLevel::INFO, std::ostringstream() << std::format("AC, {:>5}, {:>5}, {:>5}, {:>5}", "None", "TWF", "PA", "TWFPA"));
    for (int armorClass = 10; armorClass <= 30; armorClass++)
    {
        std::vector<double> damages;
        for (bool powerAttacking : {false, true})
        {
            PowerAttackResult powerAttack = calculatePowerAttack(powerAttacking, bab);
            for (bool twoWeapon : {false, true})
            {
                logger.log(LogLevel::TRACE, std::ostringstream() << "AC: " << armorClass << ", TWF: " << std::boolalpha << twoWeapon << ", PA: " << std::boolalpha << powerAttacking << ", " << powerAttack);

                std::vector<Weapon> weapons;
                weapons.emplace_back("+1 Rapier", statMod(str) + 1, DiceRoll{1, 6, statMod(str) + powerAttack.damageBonusOneHanded + 1}, 18, 2);
                logger.log(LogLevel::DEBUG, std::ostringstream() << "Weapon: " << weapons[0]);
                if (twoWeapon)
                {
                    weapons.emplace_back("Shortsword, WF, off hand", statMod(str) + weaponFocus, DiceRoll{1, 6, (statMod(str) + powerAttack.damageBonusOneHanded) / 2}, 19, 2);
                    logger.log(LogLevel::DEBUG, std::ostringstream() << "Weapon: " << weapons[1]);
                }
                damages.push_back(pathfinderAttack.averageDamage(weapons, bab, powerAttack, armorClass));
            }
        }
        std::ostringstream oss;
        oss << armorClass;
        for (const auto &damage : damages)
        {
            oss << std::format(", {:5.2f}", damage);
        }
        logger.log(LogLevel::INFO, std::ostringstream() << oss.str());
    }
}

int main()
{
    showAverageDamagePerAc();
    return 0;
}