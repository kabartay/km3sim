#!/usr/bin/env zsh

rm -f orca.gdml

km3sim \
  -i gseagen.muon-CC.3-100GeV-9.1E7-1bin-3.0gspec.ORCA115_9m_2016.1.evt \
  -o gseagen.muon-CC.3-100GeV-9.1E7-1bin-3.0gspec.ORCA115_9m_2016.1.out.evt \
  -d orca_120strings_av23min20mhorizontal_18OMs_alt9mvertical_v1.detx \
  -p INPUTParametersRun_3500_scatter_WPD3.3_p0.0075_1.0
