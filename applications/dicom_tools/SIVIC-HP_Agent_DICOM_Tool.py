import pydicom
import os 
import sys
import warnings
import datetime
warnings.filterwarnings("ignore")
#This scripts takes user inputs of contrast attributes and add those attributes into DICOM Files
#USE TERMINAL TO RUN:python3 SIVIC-HP_Agent_DICOM_Tool.py Path
#EXAMPLE: python3 SIVIC-HP_Agent_DICOM_Tool.py /Users/ernestodiaz/Desktop/RCC/Split/AUC_LACATATE 
#Creator:Ernesto Diaz

#fucntion to for the dicom attributes 
def add_dicom_attributes(ds,agent,vol,flow_duration,ingredient1,ingredient2,molarity1,molar_mass1,molarity2,molar_mass2,acq_time,date,filename):
#Adding Contrast Agent
 ds.add_new([0x0018, 0x0010], 'LO', agent)
#Adding Volume of Contrast Agent Injected
 ds.add_new([0x0018, 0x1041], 'DS', vol)
#Adding Delay Time from Start of Injection to Start of Acquisition
 ds.add_new([0x0018, 0x1042], 'TM', acq_time)
#Adding Total Injection Duration
 ds.add_new([0x0018, 0x1047], 'DS', flow_duration)
#Adding the date 
 ds.add_new([0x0400,0x0562],'DT',date)
#Adding the the filename
 ds.add_new([0x0400,0x0563],'LO',filename)
#Ingredient Concentration for ingredient1 or ingredient2
 if ingredient1 or ingredient2:
  concentration1 = float(molarity1) * (float(molar_mass1) / 1000)
  concentration2 = float(molarity1) * (float(molar_mass1) / 1000)
  #getting all the dicom attributes for the next function
  add_seq(ds,agent,vol,flow_duration,ingredient1,ingredient2,concentration1,concentration2,acq_time,date,filename)
  
#fucntion for adding the attributes to the sequence
def add_seq(ds,agent,vol,flow_duration,ingredient1,ingredient2,concentration1,concentration2,acq_time,date,filename):
  #create an empty sequence
  ds.add_new([0x0400,0x0550],'SQ',None)
  seq = ds.ModifiedAttributesSequence
  item = pydicom.Dataset()
  #then add all attributes into the modified sequence
  item.ContrastBolusAgent = agent
  item.ContrastBolusVolume = vol
  item.ContrastBolusStartTime = acq_time
  item.ContrastFlowDuration = flow_duration
  item.AttributeModificationDateTime = date
  item.ModifyingSystem = filename
  #adding the sequence if its 1 ingredient or 2 
  if ingredient1:
    item.ContrastBolusIngredient = ingredient1
    item.ContrastBolusIngredientConcentration = concentration1
  if ingredient2:
    item.ContrastBolusIngredient = [ingredient1,ingredient2]
    item.ContrastBolusIngredientConcentration = [concentration1,concentration2]
  seq +=[item]  
  #add them to the contrast boulus sequenece
  if ingredient1:
   ds.add_new([0x0018, 0x0012], 'SQ', None)
   seq = ds.ContrastBolusAgentSequence
   seq += [pydicom.Dataset()]
   seq[0].ContrastBolusIngredient = ingredient1
   seq[0].ContrastBolusIngredientConcentration = concentration1
  if ingredient2:
   ds.add_new([0x0018, 0x0012], 'SQ', None)
   seq = ds.ContrastBolusAgentSequence
   seq += [pydicom.Dataset(), pydicom.Dataset()]
   seq[0].ContrastBolusIngredient = ingredient1
   seq[0].ContrastBolusIngredientConcentration = concentration1
   seq[1].ContrastBolusIngredient = ingredient2
   seq[1].ContrastBolusIngredientConcentration = concentration2

  

#function for user input value or default value
def get_user_input_default(prompt, default):
  user_input = input(f"{prompt} (Press Enter For Default: {default}): ")
  if user_input == '':
     user_input = default
  return user_input

#function gets the tags of the dicom attributes and set its empty if its empty
def get_value(ds,tag):
  value = ds.get(tag)
  return value.value if value else ''

#you give the folder path and it loops through the path to if it's empty
def check_values(path):
    if os.path.isdir(path):
        os.chdir(path)
        files = os.listdir()
        files.sort()
        #loop
        for filename in files:
         ds = pydicom.dcmread(filename, force=True)
         agent_value = get_value(ds,(0x0018,0x0010))
         volume_value = get_value(ds,(0x0018,0x1041))
         start_value = get_value(ds,(0x0018,0x1042)) 
         flow_value = get_value(ds,(0x0018,0x1047))
         contrast_seq = get_value(ds,(0x0018,0x0012))
        #check the ingredient values but didn't really work for two
        ingredient_list = []
        if contrast_seq:
          for item in contrast_seq:
            ingredient = item.get((0x0018,0x1048))
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
    #this is for a single file
    elif os.path.isfile(path):

        ds = pydicom.dcmread(path, force=True)
        agent_value = get_value(ds,(0x0018,0x0010))
        volume_value = get_value(ds,(0x0018,0x1041))
        start_value = get_value(ds,(0x0018,0x1042)) 
        flow_value = get_value(ds,(0x0018,0x1047))
        contrast_seq = get_value(ds,(0x0018,0x0012))

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
      agent_input = input("The Current Agent is"+ " "+ agent +". "+"Would you like to overwrite it? Type Y or N:")   
      if agent_input.lower() == 'y':
        agent = get_user_input_default("Enter A Contrast Agent", "HYPERPOLARIZED [1-13C]PYRUVATE")  
   
    if not vol:
     vol = get_user_input_default("Enter The Volume of The Contrast Agent Injected In mL", "0.35")
    else:
      vol_input = input("The Current Volume is"+ " "+ str(vol) +". "+"Would you like to overwrite it? Type Y or N:")   
      if vol_input.lower() == 'y':
        vol = get_user_input_default("Enter The Volume of The Contrast Agent Injected In mL", "0.35")  
    
    if not flow:
      flow = get_user_input_default("Enter The Total Injection Duration In Seconds", "12")
    else:
      flow_input = input("The Current Total Injection Time is"+ " "+ str(flow) +". "+"Would you like overwrite it? Type Y or N:")   
      if flow_input.lower() == 'y':
        flow = get_user_input_default("Enter The Total Injection Duration In Seconds", "12")
    
    #Start needed to be in a DICOM Stadard formula
    if not start:
       start = get_user_input_default("Enter The Delay Time From Start Of Injection To Start Of Acquisition In Seconds", "0")
       start_time = int(start) %(24*3600)
       hour = start_time // 3600
       start_time %= 3600
       minutes = start_time // 60
       start_time %= 60
       start= ("%02d%02d%02d.000000" % (hour,minutes,start_time)) 
                                                                            
    else:
      start_input = input("The Current Delay Time Of The Start Acquisition"+" "+str(start)+". "+"Would you like to overwrite it? Type Y or N:") 
      if start_input.lower() =='y':
       start = get_user_input_default("Enter The Delay Time From Start Of Injection To Start Of Acquisition In Seconds", "0") 
       start_time = int(start) %(24*3600)
       hour = start_time // 3600
       start_time %= 3600
       minutes = start_time // 60
       start_time %= 60
       start = ("%02d%02d%02d.000000" % (hour,minutes,start_time)) 
       
      else:
        start

    if not ingredient1:
     ingredient1 = get_user_input_default("Enter A Contrast Ingredient", "[1-^13^C]PYRUVATE")
    else:
      ingredient1_input = input("The Current Ingredient is " + ingredient1 + ". " + "Would you like to overwrite it? Type Y or N: ")
      if ingredient1_input.lower() == 'y':
        ingredient1 = get_user_input_default("Enter A Contrast Ingredient", "[1-^13^C]PYRUVATE")
      
    molarity1 = get_user_input_default("Enter The Molarity Of The Contrast Ingredient in mM", "80")
    molar_mass1 = get_user_input_default("Enter The Molar Mass Of The Contrast Ingredient in g/mol", "88.06")


    ingredient2 = input("Would You Like To Add Another Ingredient? Type Y or N:")
    if ingredient2.lower() == 'y':
     ingredient2 = get_user_input_default("Enter Another Ingredient", "[^13^C,^15^N]UREA")
     molarity2 = get_user_input_default("Enter The Molarity For The Second Contrast Ingredient in mM", "100")
     molar_mass2 = get_user_input_default("Enter The Molar Mass For The Second Contrast Ingredient in g/mol", "60.06")
    else:
     ingredient2 = None
     molarity2 = None
     molar_mass2 = None
   
    now = datetime.datetime.now()
    modifi_date = now.strftime("%Y%m%d%H")
    filename = os.path.basename(__file__)

    # Add DICOM attributes to all files in the specified directory
    if os.path.isdir(path):
        os.chdir(path)
        files = os.listdir()
        files.sort()

        for path in files:
            ds = pydicom.dcmread(path,force=True)
            add_dicom_attributes(ds,agent,vol,flow,ingredient1,ingredient2,molarity1,molar_mass1,molarity2,molar_mass2,start,modifi_date,filename)
            print(ds.filename) 
            ds.save_as(path)
            print("These are the attributes that you modified:")
            print(ds.ModifiedAttributesSequence)
           
    else:
            ds = pydicom.dcmread(path,force=True)
            add_dicom_attributes(ds,agent,vol,flow,ingredient1,ingredient2,molarity1,molar_mass1,start,modifi_date,filename)
            print(ds.filename)
            ds.save_as(path)
            print("These are the attributes that you modified:")
            print(ds.ModifiedAttributesSequence)
check_values(sys.argv[1])