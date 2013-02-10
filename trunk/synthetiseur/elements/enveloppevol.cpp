/***************************************************************************
**                                                                        **
**  Polyphone, a soundfont editor                                         **
**  Copyright (C) 2013 Davy Triponney                                     **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Davy Triponney                                       **
**  Website/Contact: http://www.polyphone.fr/                             **
**             Date: 01.01.2013                                           **
***************************************************************************/

#include "enveloppevol.h"

EnveloppeVol::EnveloppeVol(VoiceParam * voiceParam, quint32 sampleRate, bool isMod) :
    m_currentSmpl(0),
    m_precValue(0),
    m_currentPhase(phase1delay),
    m_sampleRate(sampleRate)
{
    // Stockage des paramètres initiaux
    if (isMod)
    {
        this->m_timeDelay       = voiceParam->modDelayTime * m_sampleRate;
        this->m_timeAttack      = voiceParam->modAttackTime * m_sampleRate;
        this->m_timeHold        = voiceParam->modHoldTime * m_sampleRate;
        this->m_timeDecay       = voiceParam->modDecayTime * m_sampleRate;
        this->m_levelSustain    = voiceParam->modSustainLevel;
        this->m_timeRelease     = voiceParam->modReleaseTime * m_sampleRate;
        this->m_noteToHold      = (double)voiceParam->modKeynumToHold / 1200;
        this->m_noteToDecay     = (double)voiceParam->modKeynumToDecay / 1200;
        this->m_volume          = 0;
        this->m_fixedVelocity   = voiceParam->fixedVelocity;
    }
    else
    {
        this->m_timeDelay       = voiceParam->volDelayTime * m_sampleRate;
        this->m_timeAttack      = voiceParam->volAttackTime * m_sampleRate;
        this->m_timeHold        = voiceParam->volHoldTime * m_sampleRate;
        this->m_timeDecay       = voiceParam->volDecayTime * m_sampleRate;
        this->m_levelSustain    = voiceParam->volSustainLevel;
        this->m_timeRelease     = voiceParam->volReleaseTime * m_sampleRate;
        this->m_noteToHold      = (double)voiceParam->volKeynumToHold / 1200;
        this->m_noteToDecay     = (double)voiceParam->volKeynumToDecay / 1200;
        this->m_volume          = -voiceParam->attenuation;
        this->m_fixedVelocity   = voiceParam->fixedVelocity;
    }
}

bool EnveloppeVol::applyEnveloppe(qint32 * data, quint32 size, bool release, int note,
                                  int velocity, VoiceParam * voiceParam,
                                  double gain, qint32 valFixe)
{
    // Pour l'instant pas de mise à jour en temps réel des paramètres
    Q_UNUSED(voiceParam);
    // Application de l'enveloppe sur des données
    // renvoie 1 si la fin de la release est atteint
    if (release && m_currentPhase != phase7off && m_currentPhase != phase6release)
    {
        m_currentPhase = phase6release;
        m_currentSmpl = 0;
    }
    // Ajustement sustain level
    double levelSustain = 0;
    if (valFixe == 0)
        levelSustain = pow(10, -m_levelSustain / 50); // dB
    else
        levelSustain = 1. - m_levelSustain / 100; // %
    // Ajustement hold time et volume
    quint32 timeHold = m_timeHold * pow(2, m_noteToHold * (60 - note));
    quint32 timeDecay = m_timeDecay * pow(2, m_noteToDecay * (60 - note));
    double volume = pow(10, (m_volume + gain) / 50); // normalement division par 10
    if (m_fixedVelocity != -1)
        volume *= (double)(m_fixedVelocity * m_fixedVelocity) / 16129;
    else
        volume *= (double)(velocity * velocity) / 16129;
    // Avancement
    bool fin = false;
    quint32 avancement = 0;
    quint32 duree = 0;
    double val2 = 0;
    double coef, valTmp;
    while (avancement < size)
    {
        switch (m_currentPhase)
        {
        case phase1delay:
            // Nombre de valeurs restantes dans la phase
            duree = m_timeDelay - m_currentSmpl;
            if (duree <= size - avancement)
            {
                m_currentPhase = phase2attack;
                m_currentSmpl = 0;
            }
            else
            {
                duree = size - avancement;
                m_currentSmpl += duree;
            }
            val2 = 0;
            for (quint32 i = 0; i < duree; i++)
                data[avancement + i] = 0;
            break;
        case phase2attack:
            // Nombre de valeurs restantes dans la phase
            duree = m_timeAttack - m_currentSmpl;
            if (duree <= size - avancement)
            {
                m_currentPhase = phase3hold;
                m_currentSmpl = 0;
                val2 = 1;
            }
            else
            {
                duree = size - avancement;
                m_currentSmpl += duree;
                val2 = (double)m_currentSmpl / m_timeAttack;
            }
            // Attaque convexe => amplitude linéaire
            coef = (val2 - this->m_precValue) / duree;
            valTmp = 0;
            if (valFixe == 0)
            {
                for (quint32 i = 0; i < duree; i++)
                {
                    data[avancement + i] = volume * ((double)data[avancement + i] *
                                                     (valTmp + this->m_precValue));
                    valTmp += coef;
                }
            }
            else
            {
                for (quint32 i = 0; i < duree; i++)
                {
                    data[avancement + i] = volume * ((double)valFixe *
                                                     (valTmp + this->m_precValue));
                    valTmp += coef;
                }
            }
            break;
        case phase3hold:
            // Nombre de valeurs restantes dans la phase
            duree = timeHold - m_currentSmpl;
            if (duree <= size - avancement)
            {
                m_currentPhase = phase4decay;
                m_currentSmpl = 0;
            }
            else
            {
                duree = size - avancement;
                m_currentSmpl += duree;
            }
            val2 = 1;
            if (valFixe == 0)
            {
                for (quint32 i = 0; i < duree; i++)
                    data[avancement + i] = volume * (double)data[avancement + i];
            }
            else
            {
                for (quint32 i = 0; i < duree; i++)
                    data[avancement + i] = volume * (double)valFixe;
            }
            break;
        case phase4decay:
            // Nombre de valeurs restantes dans la phase
            duree = timeDecay - m_currentSmpl;
            val2 = levelSustain;
            if (duree <= size - avancement)
            {
                m_currentPhase = phase5sustain;
                m_currentSmpl = 0;
            }
            else
            {
                duree = size - avancement;
                m_currentSmpl += duree;
                val2 = 1. - ((double)m_currentSmpl / timeDecay) * (1 - val2);
            }
            if (valFixe == 0)
            {
                // Décroissance exponentielle
                coef = pow(levelSustain, 1. / timeDecay);
                val2 = m_precValue;
                for (quint32 i = 0; i < duree; i++)
                {
                    data[avancement + i] = volume * ((double)data[avancement + i] * val2);
                    val2 *= coef;
                }
            }
            else
            {
                // Linéaire
                coef = (val2 - this->m_precValue) / duree;
                valTmp = 0;
                for (quint32 i = 0; i < duree; i++)
                {
                    data[avancement + i] = volume * ((double)valFixe *
                                                     (valTmp + this->m_precValue));
                    valTmp += coef;
                }
            }
            break;
        case phase5sustain:
            // Nombre de valeurs
            duree = size - avancement;
            val2 = levelSustain;
            if (valFixe == 0)
            {
                for (quint32 i = 0; i < duree; i++)
                    data[avancement + i] = volume * ((double)data[avancement + i] * val2);
            }
            else
            {
                for (quint32 i = 0; i < duree; i++)
                    data[avancement + i] = volume * ((double)valFixe * val2);
            }
            break;
        case phase6release:
            // Nombre de valeurs restantes dans la phase
            duree = m_timeRelease - m_currentSmpl;
            if (duree <= size - avancement)
            {
                m_currentPhase = phase7off;
                m_currentSmpl = 0;
                val2 = 0;
            }
            else
            {
                duree = size - avancement;
                m_currentSmpl += duree;
                val2 = m_precValue * (1. - ((double)m_currentSmpl / m_timeRelease));
            }
            if (valFixe == 0)
            {
                // Décroissance exponentielle
                coef = pow(0.01, 1. / m_timeRelease);
                val2 = m_precValue;
                for (quint32 i = 0; i < duree; i++)
                {
                    data[avancement + i] = volume * ((double)data[avancement + i] * val2);
                    val2 *= coef;
                }
            }
            else
            {
                // Linéaire
                valTmp = 0;
                coef = (val2 - this->m_precValue) / duree;
                for (quint32 i = 0; i < duree; i++)
                {
                    data[avancement + i] = volume * ((double)valFixe *
                                                     (valTmp + this->m_precValue));
                    valTmp += coef;
                }
            }
            break;
        case phase7off:
            // Nombre de valeurs
            duree = size - avancement;
            val2 = 0;
            for (quint32 i = 0; i < duree; i++)
                data[avancement + i] = 0;
            // Fin
            fin = true;
        }
        // On retient la dernière valeur et on avance
        this->m_precValue = val2;
        avancement += duree;
    }
    return fin;
}