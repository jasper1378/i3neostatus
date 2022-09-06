/*
 * https://github.com/i3/i3status
 * Copyright © 2009, Michael Stapelberg and contributors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of i3status nor the names of contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#ifndef PULSE_HPP
#define PULSE_HPP

#include <stdbool.h>
#include <stdint.h>

#define APP_NAME "jasper.i3blocks.volume.pulse"
#define APP_ID "jasper.i3blocks.volume.pulse"
#define APP_VERSION "0"

#define DEFAULT_SINK_INDEX UINT32_MAX
#define COMPOSE_VOLUME_MUTE(vol, mute) ((vol) | ((mute) ? (1 << 30) : 0))
#define DECOMPOSE_VOLUME(cvol) ((cvol) & ~(1 << 30))
#define DECOMPOSE_MUTED(cvol) (((cvol) & (1 << 30)) != 0)
#define MAX_SINK_DESCRIPTION_LEN (128) /* arbitrary */

#define PULSEAUDIO_VOLUME_FAIL -1
#define PULSEAUDIO_DESCRIPTION_FAIL "FAIL"

int volume_pulseaudio(uint32_t sink_idx, const char *sink_name);
bool description_pulseaudio(uint32_t sink_idx, const char *sink_name, char buffer[MAX_SINK_DESCRIPTION_LEN]);
bool pulse_initialize(void);

#endif
