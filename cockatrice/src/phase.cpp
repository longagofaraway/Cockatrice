#include "phase.h"

Phase::Phase(const char *_name, QString _color, QString _soundFileName) : color(_color), soundFileName(_soundFileName)
{
    name = tr(_name);
}

Phase Phases::getPhase(int phase)
{
    if (0 <= phase && phase < Phases::phaseTypesCount) {
        return phases[phase];
    } else {
        return unknownPhase;
    }
}

const Phase Phases::unknownPhase(QT_TRANSLATE_NOOP("Phase", "Unknown Phase"), "black", "unknown_phase");
const Phase Phases::phases[Phases::phaseTypesCount] = {
    {QT_TRANSLATE_NOOP("Phase", "Untap"), "green", "untap_step"},
    {QT_TRANSLATE_NOOP("Phase", "Draw"), "green", "upkeep_step"},
    {QT_TRANSLATE_NOOP("Phase", "Clock"), "green", "draw_step"},
    {QT_TRANSLATE_NOOP("Phase", "Main"), "blue", "main_1"},
    {QT_TRANSLATE_NOOP("Phase", "Attack Declaration"), "red", "start_combat"},
    {QT_TRANSLATE_NOOP("Phase", "Trigger"), "red", "attack_step"},
    {QT_TRANSLATE_NOOP("Phase", "Counter"), "red", "block_step"},
    {QT_TRANSLATE_NOOP("Phase", "Damage"), "red", "damage_step"},
    {QT_TRANSLATE_NOOP("Phase", "Batle"), "red", "end_combat"},
    {QT_TRANSLATE_NOOP("Phase", "Encore"), "green", "end_step"}};
