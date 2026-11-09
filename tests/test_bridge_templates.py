from pathlib import Path
import yaml,json
from jinja2 import Environment
root=Path(__file__).resolve().parents[1]
d=yaml.safe_load((root/'ha/hmh_knob_scripts.yaml').read_text())
tpl=d['hmh_knob_catalog']['sequence'][0]['variables']['result']['payload']
st=d['hmh_knob_state']['sequence'][0]['variables']['result']['payload']
e=Environment(autoescape=False,trim_blocks=True,lstrip_blocks=True)
e.filters['to_json']=lambda x:json.dumps(x,ensure_ascii=False)
# Fixture: physical smart plug with LED indicator + relay and multiple measurement sensors
registry={
'dev_plug': ['light.colonna_camera_led_blu','switch.colonna_camera_relay','sensor.colonna_camera_power','sensor.colonna_camera_voltage','sensor.colonna_camera_current','sensor.colonna_camera_power_factor'],
'dev_lamp': ['light.lampada_soggiorno','switch.lampada_soggiorno_power'],
'dev_clima': ['climate.termostato_virtuale','sensor.clima_temperatura','sensor.clima_umidita'],
'dev_cover': ['cover.tenda_sala','sensor.tenda_batteria'],
'dev_sensors': ['sensor.temperatura_esterna','sensor.umidita_esterna'],
'dev_led_override': ['light.led_blu','switch.led_override_relay'],
}
meta={
'light.colonna_camera_led_blu':dict(friendly_name='ESPHome Colonna Camera LED Blu',supported_color_modes=['onoff']),
'switch.colonna_camera_relay':dict(friendly_name='ESPHome Colonna Camera Relè'),
'sensor.colonna_camera_power':dict(device_class='power',unit_of_measurement='W'),
'sensor.colonna_camera_voltage':dict(device_class='voltage',unit_of_measurement='V'),
'sensor.colonna_camera_current':dict(device_class='current',unit_of_measurement='A'),
'sensor.colonna_camera_power_factor':dict(device_class='power_factor'),
'light.lampada_soggiorno':dict(friendly_name='Lampada Soggiorno',supported_color_modes=['hs'],brightness=200),
'switch.lampada_soggiorno_power':dict(friendly_name='Lampada Soggiorno Power'),
'climate.termostato_virtuale':dict(friendly_name='Termostato',target_temp_step=0.5,temperature=22, current_temperature=21, hvac_modes=['heat','off'], supported_features=1),
'cover.tenda_sala':dict(friendly_name='Tenda Sala',supported_features=15),
'sensor.temperatura_esterna':dict(friendly_name='Temperatura Esterna',device_class='temperature'),
'sensor.umidita_esterna':dict(friendly_name='Umidita Esterna',device_class='humidity'),
'light.led_blu':dict(friendly_name='LED Blu',supported_color_modes=['onoff']),
'switch.led_override_relay':dict(friendly_name='Relè'),
}
state={
'switch.colonna_camera_relay':'on','light.colonna_camera_led_blu':'on',
'sensor.colonna_camera_power':'246.9','sensor.colonna_camera_voltage':'229.1',
'sensor.colonna_camera_current':'1.52','sensor.colonna_camera_power_factor':'0.71',
}
owner={item:dev for dev,items in registry.items() for item in items}
explicit=['light.led_blu','scene.serata']
labeled=['dev_plug','dev_lamp','dev_clima','dev_cover','dev_sensors','dev_led_override']
e.globals.update(
  label_entities=lambda x:explicit,
  label_devices=lambda x:labeled,
  device_id=lambda x:owner.get(x),
  device_entities=lambda d:registry[d],
  device_name=lambda d: {'dev_plug':'ESPHOME Colonna Camera','dev_lamp':'Lampada Soggiorno','dev_clima':'Riscaldamento Sala','dev_cover':'Tenda Sala','dev_sensors':'Sonde Esterno','dev_led_override':'LED Test'}[d],
  state_attr=lambda ent,attr:meta.get(ent,{}).get(attr),
  states=lambda ent:state.get(ent,'off'),
)
output=json.loads(e.from_string(tpl).render())
for item in output:print(item['name'],item['entity'],item['domain'])
byname={v['name']:v for v in output}
assert len(output)==7,len(output)
assert byname['ESPHOME Colonna Camera']['entity']=='switch.colonna_camera_relay'
assert byname['Lampada Soggiorno']['entity']=='light.lampada_soggiorno'
assert byname['Riscaldamento Sala']['entity']=='climate.termostato_virtuale'
assert byname['Tenda Sala']['entity']=='cover.tenda_sala'
assert byname['LED Test']['entity']=='light.led_blu' # explicitly tagged entity overrides rank
assert byname['Sonde Esterno']['domain']=='sensor'
assert byname['serata' if 'serata' in byname else 'scene.serata']['domain']=='scene'
stateoutput=json.loads(e.from_string(st).render(entity_id='switch.colonna_camera_relay'))
assert stateoutput['power']=='246.9',stateoutput
assert stateoutput['power_factor']=='0.71',stateoutput
print('ALL TESTS OK: count, selected entity, direct label override, sensors, climate, covers, power factor')
