#pragma once

unsigned int coded_mb_size();

void RBSP_decode(NALunit nal_unit);
void RBSP_encode(NALunit &nal_unit);