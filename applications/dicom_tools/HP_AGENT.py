#HP13 DICOM Adding Attributes 
#This script asks user inputs of Contrast Agents and will add those Agents to DICOM Files.
#Ernesto Diaz 
import sys
import os
import pydicom
   
    
def main(path):
 
 print("----Welcome----\n")

#Add Contrast Agent
 agent = input("Enter A Contrast Agent Or Press Enter For Default[Pyruvate]:")
 if agent =='':   
  agent = 'pyruvate'
#Add Volume Agent
 vol = input("Enter The Volume For The Contrast Agent Or Press Enter For Default[0.35mL]:")
 if vol =='':
  vol ='0.35'
#Add Start Time
 start_time = input("Enter The Delay Time From Start Of Injection To Start Of Acquisition Or Press Enter For Default[0sec]:")
 if start_time == '':
  start_time = '0' 
#Add Total Injection Time
 total_injection_time = input("Enter The Total Injection Duration Or Press Enter For Default[12sec]:")
 if total_injection_time == '':
  total_injection_time = '12'
#Add Ingredient 
 ingredient = input("Enter A Contrast Ingredient Or Press Enter For Default[Pyruvate]:")
 if ingredient == '':
  ingredient = "Pyruvate"
#Adding Concentration But Using Molarity and Molar Mass
 molarity = input("Enter The Molarity Of The Contrast Ingredient Or Press Enter For Default[80mM]")
 if molarity == '':
  molarity = '80'
  float(molarity)
 molar_mass = input("Enter The Molar Mass Of The Contrast Ingredient Press Enter For Default[88.06g/mol]")
 if molar_mass == '':
   molar_mass = '88.06'
   float(molar_mass)
 concentration = float(molarity) * (float(molar_mass)/1000)
# print("The Concentration Ingredient is:"+ str(concentration) +"mg/ml is the ingredient concentration \n")

#Change the folder to the path 
 if os.path.isdir(path):
  os.chdir(path)
 #Change The Folder Path To A List and sort it in numerical orders
  files = os.listdir()
  files.sort()

  for path in files:
     ds = pydicom.dcmread(path)
     ds.add_new([0x0018,0x0010],'LO',agent)
     ds.add_new([0x0018,0x1041],'DS',vol)
     ds.add_new([0x0018,0x1048],'CS',ingredient)
     ds.add_new([0x0018,0x1042],'TM',start_time)
     ds.add_new([0x0018,0x1047],'DS',total_injection_time) 
     ds.add_new([0x0018,0x1049],'DS',concentration)
     ds.save_as(path)
     
     #Uncomment this if you want to see the results in the DICOM files 
     print(ds)
 else:
    ds2 = pydicom.dcmread(path)
    ds2.add_new([0x0018,0x0010],'LO',agent)
    ds2.add_new([0x0018,0x1041],'DS',vol)
    ds2.add_new([0x0018,0x1048],'CS',ingredient)
    ds2.add_new([0x0018,0x1042],'TM',start_time)
    ds2.add_new([0x0018,0x1047],'DS',total_injection_time) 
    ds2.add_new([0x0018,0x1049],'DS',concentration)
    ds2.save_as(path)

 print("All Agents Has Been Added")
main(sys.argv[1])     


 
