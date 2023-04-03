import pydicom
import os 
import sys
import warnings
warnings.filterwarnings("ignore")
#This scripts takes user inputs of contrast attributes and add those attributes into DICOM Files
#USE TERMINAL TO RUN:python3 SIVIC-HP_Agent_DICOM_Tool.py Path
#EXAMPLE: python3 SIVIC-HP_Agent_DICOM_Tool.py /Users/ernestodiaz/Desktop/RCC/Split/AUC_LACATATE 
#Ernesto Diaz

#fucntion to for the dicom attributes 
def add_dicom_attributes(ds, agent, vol,flow_duration, ingredient1, ingredient2, molarity, molar_mass,acq_time):
#Adding Contrast Agent
 ds.add_new([0x0018, 0x0010], 'LO', agent)
#Adding Volume of Contrast Agent Injected
 ds.add_new([0x0018, 0x1041], 'DS', vol)
#Adding Delay Time from Start of Injection to Start of Acquisition
 ds.add_new([0x0018, 0x1042], 'TM', acq_time)
#Adding Total Injection Duration
 ds.add_new([0x0018, 0x1047], 'DS', flow_duration)
#Adding Concentration of Contrast Ingredient
 concentration = float(molarity) * (float(molar_mass) / 1000)
 ds.add_new([0x0018, 0x1049], 'DS', concentration)
#Adding Ingredient  
 if ingredient1:
  ds.add_new([0x0018,0x0012],'SQ',None)
  seq = ds.ContrastBolusAgentSequence
  seq += [pydicom.Dataset()]
  seq[0].ContrastBolusIngredient = ingredient1
 if ingredient2: 
  ds.add_new([0x0018,0x0012],'SQ',None)
  seq = ds.ContrastBolusAgentSequence
  seq += [pydicom.Dataset(),pydicom.Dataset()]
  seq[0].ContrastBolusIngredient = ingredient1
  seq[1].ContrastBolusIngredient = ingredient2

#function for user input value or default value
def get_user_input_default(prompt, default):
  user_input = input(f"{prompt} (Press Enter For Default: {default}): ")
  if user_input == '':
     user_input = default
  return user_input

#function for checking values if the DICOM Files are valid, if not will mark the attributes as empty
def check_values(path):
    if os.path.isdir(path):
        os.chdir(path)
        files = os.listdir()
        files.sort()
        
        for filename in files:
         ds = pydicom.dcmread(filename, force=True)
         contrast_agent = ds.get((0x0018, 0x0010))
         volume = ds.get((0x0018, 0x1041))
         start_time = ds.get((0x0018, 0x1042))
         flow_duration = ds.get((0x0018, 0x1047))
         contrast_seq = ds.get((0x0018,0x0012))
         ingredient = ds.get((0x0018,0x1048))

      
         if contrast_agent:
          agent_value = contrast_agent.value
         else:
          agent_value = ''
         if volume:
          volume_value = volume.value
         else:
          volume_value = ''
         if flow_duration:
          flow_value = flow_duration.value
         else:
          flow_value = ''
         if start_time:
          start_value = start_time.value
         else:
          start_value = ''   
        
        ingredient_list = []
        if contrast_seq:
          for item in contrast_seq:
            ingredient= item.get((0x0018,0x1048))
            if ingredient:
              ingredient_value = ingredient.value
            if ingredient_value:
              ingredient_list.append(ingredient_value)
          if len(ingredient_list) >=2:
            first_ingredient = ingredient_list[0]
            second_ingredient = ingredient_list[1]
          elif len(ingredient_list)==1:
            first_ingredient = ingredient_list[0]
            second_ingredient = ''
          else:
            first_ingredient = None
            second_ingredient = None
        else:
          first_ingredient = None
          second_ingredient = None    

      
        main(path,agent_value,volume_value,start_value,flow_value,first_ingredient,second_ingredient)
    
    elif os.path.isfile(path):
        ds = pydicom.dcmread(path, force=True)
        contrast_agent = ds.get((0x0018, 0x0010))
        volume = ds.get((0x0018, 0x1041))
        start_time = ds.get((0x0018, 0x1042))
        flow_duration = ds.get((0x0018, 0x1047))
        contrast_seq = ds.get((0x0018,0x0012))
        ingredient = ds.get((0x0018,0x1048))

        if contrast_agent:
            agent_value = contrast_agent.value
        else:
            agent_value = ''
        if volume:
            volume_value = volume.value
        else:
            volume_value = ''
        if flow_duration:
            flow_value = flow_duration.value
        else:
            flow_value = ''
        if start_time:
            start_value = start_time.value
        else:
            start_value = ''   
        
        ingredient_list = []
        if contrast_seq:
          for item in contrast_seq:
            ingredient= item.get((0x0018,0x1048))
            if ingredient:
              ingredient_value = ingredient.value
            if ingredient_value:
              ingredient_list.append(ingredient_value)
          if len(ingredient_list) >=2:
            first_ingredient = ingredient_list[0]
            second_ingredient = ingredient_list[1]
          elif len(ingredient_list)==1:
            first_ingredient = ingredient_list[0]
            second_ingredient = ''
          else:
            first_ingredient = None
            second_ingredient = None
        else:
          first_ingredient = None
          second_ingredient = None    

        main(path,agent_value,volume_value,start_value,flow_value,first_ingredient,second_ingredient)
    else:
        print("Error: Path is neither a directory nor a file.")

#main function where we ask for user inputs and add the attributes to dicom path
def main(path,agent,vol,start,flow,ingredient1,ingredient2):
    print("----Welcome----\n")
    if not agent:
      agent = get_user_input_default("Enter A Contrast Agent", "HYPERPOLARIZED [1-13C]PYRUVATE")
    else:
      agent_input = input("The Current Agent is"+ " "+ agent +". "+"Would you like to overwrite it? Type Yes or No:")   
      if agent_input.lower() == 'yes':
        agent = get_user_input_default("Enter A Contrast Agent", "HYPERPOLARIZED [1-13C]PYRUVATE")  
   
    if not vol:
     vol = get_user_input_default("Enter The Volume of The Contrast Agent Injected In mL", "0.35")
    else:
      vol_input = input("The Current Volume is"+ " "+ str(vol) +". "+"Would you like to overwrite it? Type Yes or No:")   
      if vol_input.lower() == 'yes':
        vol = get_user_input_default("Enter The Volume of The Contrast Agent Injected In mL", "0.35")  
    
    if not flow:
      flow = get_user_input_default("Enter The Total Injection Duration In Seconds", "12")
    else:
      flow_input = input("The Current Total Injection Time is"+ " "+ str(flow) +". "+"Would you like overwrite it? Type Yes or No:")   
      if flow_input.lower() == 'yes':
        flow = get_user_input_default("Enter The Total Injection Duration In Seconds", "12")

    if not start:
       start = get_user_input_default("Enter The Delay Time From Start Of Injection To Start Of Acquisition In Seconds", "0")
       start_time = int(start) %(24*3600)
       hour = start_time // 3600
       start_time %= 3600
       minutes = start_time // 60
       start_time %= 60
       acq_time = ("%02d%02d%02d.000000" % (hour,minutes,start_time)) 
       str(acq_time)
    else:
      start_input = input("The Current Delay Time Of The Start Acquisition"+" "+str(start)+". "+"Would you like to overwrite it? Type Yes or No:") 
      if start_input.lower() =='yes':
       start = get_user_input_default("Enter The Delay Time From Start Of Injection To Start Of Acquisition In Seconds", "0") 
       start_time = int(start) %(24*3600)
       hour = start_time // 3600
       start_time %= 3600
       minutes = start_time // 60
       start_time %= 60
       acq_time = ("%02d%02d%02d.000000" % (hour,minutes,start_time)) 
       str(acq_time)
      else: 
        acq_time = start    
  
    if not ingredient1:
     ingredient1 = get_user_input_default("Enter A Contrast Ingredient", "[1-^13^C]PYRUVATE")
    else:
      ingredient1_input = input("The Current Ingredient is " + ingredient1 + ". " + "Would you like to overwrite it? Type Yes or No: ")
      if ingredient1_input.lower() == 'yes':
        ingredient1 = get_user_input_default("Enter A Contrast Ingredient", "[1-^13^C]PYRUVATE")

    ingredient2 = input("Would You Like To Add Another Ingredient? Type Yes or No:")
    if ingredient2.lower() == 'yes':
     ingredient2 = get_user_input_default("Enter Another Ingredient", "[^13^C,^15^N]UREA")
    else:
     ingredient2 = None
    
    molarity = get_user_input_default("Enter The Molarity Of The Contrast Ingredient in mM", "80")
    molar_mass = get_user_input_default("Enter The Molar Mass Of The Contrast Ingredient in g/mol", "88.06")

    # Add DICOM attributes to all files in the specified directory
    if os.path.isdir(path):
        os.chdir(path)
        files = os.listdir()
        files.sort()

        for path in files:
            ds = pydicom.dcmread(path,force=True)
            add_dicom_attributes(ds,agent,vol,flow,ingredient1,ingredient2,molarity,molar_mass,start)
            print(ds.filename) 
            ds.save_as(path)
            print(ds)
            print(f"{path} modified.")
           
    else:
            ds = pydicom.dcmread(path,force=True)
            add_dicom_attributes(ds,agent,vol,flow,ingredient1,ingredient2,molarity,molar_mass,start)
            print(ds.filename)
            ds.save_as(path)
            print(ds)
            print(f"{path} modified.")
check_values(sys.argv[1])