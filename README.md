# reram-ynthesis
A synthesis flow for hybrid processing-in-ReRAM modes

This project contains three parts:
## digital-synthesis: a synthesis flow for the digital mode (stateful logic)
* function: synthesize a logic function onto a digital ReRAM crossbar
* compiling: cd digital-synthesis/abc; make libabc.a; cd ..; make
* usage: identify the benchmark in main.cpp to get its latency and area
* todo: integrate the technology mapping flow

## hybrid-synthesis: a synthesis flow for the digital mode and the analog mode (multiply-and-accumulate)
* function: synthesize an application in the skeleton representation onto the target architecture with hybrid ReRAM crossbars
* compiling: cd hybrid_synthesis; make
* usage: write the application using nested skeletons in main.cpp to get its latency and bounding box
* todo: integrate the digital synthesis flow and support the analog mode

## simulator: a simulator for ReRAM crossbars
* function: simulate an ReRAM micro-instruction sequence and evaluate its performance
* compiling: cd simulator; make
* usage: write the application using pre-defined kernels in main.cpp to get its latency, area, energy, etc.
* todo: support more stateful logic families and more processing-in-ReRAM modes

## reference: 

```
@inproceedings{wang2019parallel,
  title={Parallel Stateful Logic in RRAM: Theoretical Analysis and Arithmetic Design},
  author={Wang, Feng and Luo, Guojie and Sun, Guangyu and Zhang, Jiaxi and Huang, Peng and Kang, Jinfeng},
  booktitle=ASAP,
  volume={2160},
  pages={157--164},
  year={2019},
  organization={IEEE}
}

@inproceedings{wang2020dual,
  title={Dual-Output LUT Merging during FPGA Technology Mapping},
  author={Wang, Feng and Zhu, Liren and Zhang, Jiaxi and Li, Lei and Zhang, Yang and Luo, Guojie},
  booktitle=ICCAD,
  pages={1--9},
  year={2020},
  organization={IEEE}
}

@article{wang2020star,
  title={{STAR}: Synthesis of Stateful Logic in {RRAM} Targeting High ARea Utilization},
  author={Wang, Feng and Luo, Guojie and Sun, Guangyu and Zhang, Jiaxi and Kang, Jinfeng and Wang, Yuhao and Niu, Dimin and Zheng, Hongzhong},
  journal=IEEE-CAD,
  year={2020},
  volume={40},
  number={5},
  pages={864--877},
  publisher={IEEE}
}
```