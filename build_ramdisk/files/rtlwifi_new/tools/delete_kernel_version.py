#!/usr/bin/python
import os

#please download and make unifdef-2.10 first
#config
root_dir = "./mac80211V643"
current_linux_ver = '-DLINUX_VERSION_CODE=0x031400'
unifdef_path = '/home/troy/check/unifdef/unifdef'



def compute_kernel_version(a,b,c):
	res = '0x'
 	if len(a) == 1:
		res = res + '0'
	res = res + a
 	if len(b) == 1:
		res = res + '0'
	res = res + b
 	if len(c) == 1:
		res = res + '0'
	res = res + c
	return res
	





def start_process(pathin):

    

	filehandler = open(pathin,'r')#open old
 	filewhandler = open(pathin+"_new",'w')#generate new
	filehandler.seek(0)  
	textlist = filehandler.readlines()  
	for line in textlist:
		if 'KERNEL_VERSION' in line:

			line = line.replace(' ','')
			#print line
			v_index = line.index('KERNEL_VERSION')
			if(v_index <= 0):
				print 'error occured!'
				return;
			v_index1 = v_index+len('KERNEL_VERSION(')
			digit_index_1 = line.index(',');#search the first ,
			digit1 = line[v_index1:digit_index_1]#get the first digit
			#print digit1
			digit_index_2 = line.index(',',digit_index_1+1);#search the second ,
			digit2 = line[digit_index_1+1:digit_index_2]#get the second digit
			#print digit2
			digit_index_3 = line.index(')',digit_index_2+1);#search the )
			digit3 = line[digit_index_2+1:digit_index_3]#get the third digit
			#print digit3
			version_res = compute_kernel_version(digit1,digit2,digit3)

			line = line.replace(line[v_index:digit_index_3+1],version_res)
		filewhandler.write(line)

	filehandler.close()
	filewhandler.close()





def Mystart(root_dir):   

	for lists in os.listdir(root_dir):
 		path = os.path.join(root_dir, lists)
		if(".c" in lists or ".h" in lists):
			start_process(path);#generate new
			unidef_command = unifdef_path+' '+current_linux_ver+' '+path+"_new"+" > "+path
			os.system(unidef_command)
			os.remove(path+"_new");#delete new
		if os.path.isdir(path):
			Mystart(path)









#start

Mystart(root_dir)

 
