// Offline regression of the v0.1.3 encoder and interaction state machine.
// This does NOT substitute for compiling ESPHome or testing actual hardware.
#include <cassert>
#include <iostream>
#include <cstdint>
uint32_t fake_millis = 0;
#include "../esphome/hmh_universal_knob.h"

int main() {
  HMHUniversalKnob k;
  HMHUniversalKnob::Device a, b, c;
  a.entity="switch.one"; a.domain="switch";
  b.entity="switch.two"; b.domain="switch";
  c.entity="switch.three"; c.domain="switch";
  k.devices={a,b,c};
  assert(k.index==0 && k.ui_mode==0);
  assert(k.rotate(1, 1000, 180, 6500) && k.index==1);
  assert(!k.rotate(1, 1030, 180, 6500) && k.index==1); // bounce
  assert(!k.rotate(-1, 1150, 180, 6500) && k.index==1);
  assert(k.rotate(1, 1181, 180, 6500) && k.index==2);
  k.click(1250);
  assert(k.ui_mode==1 && k.index==2);
  k.note_interaction(1300); // button-down protects active page from timeout
  assert(!k.return_to_browse_if_idle(7799,6500));
  assert(k.ui_mode==1);
  assert(k.return_to_browse_if_idle(7800,6500));
  assert(k.ui_mode==0 && k.field_index==0 && k.index==2);
  assert(k.rotate(1,7801,180,6500) && k.index==0); // immediate first detent

  // Adjusting an unconfirmed parameter must never issue a command.
  k.devices[0].domain="light"; k.devices[0].dim=true;
  k.live.entity="light.one"; k.live.status="on";
  k.live.valid=true; k.live.brightness=50;
  k.click(9000);
  assert(k.ui_mode==1 && k.field()=="POWER");
  assert(k.rotate(1,9010,180,6500) && k.field()=="BRIGHTNESS");
  k.click(9100);
  assert(k.ui_mode==2);
  assert(k.rotate(1,9200,180,6500));
  assert(k.draft==55);
  assert(!k.pending);
  assert(k.return_to_browse_if_idle(15700,6500));
  assert(k.ui_mode==0 && !k.pending && k.index==0);

  // Incoming HA reads do not reset the timer.
  k.click(20000);
  k.last_state_ms=26000; k.last_catalog_ms=26001;
  assert(k.return_to_browse_if_idle(26500,6500));

  // A detent after expiration but before the interval tick browses, not edits.
  k.click(30000);
  assert(k.ui_mode==1);
  assert(k.rotate(1,37000,180,6500) && k.ui_mode==0 && k.index==1);

  // Custom timeout is observed in both the timer and rotate paths.
  k.click(40000);
  assert(k.ui_mode==1);
  assert(k.rotate(1,47000,180,10000) && k.ui_mode==1);
  assert(!k.return_to_browse_if_idle(57000,15000));
  assert(k.return_to_browse_if_idle(62000,15000));

  std::cout << "PASS: detent throttle, auto-return, no surprise commands, first-step, custom timeout\n";
}
