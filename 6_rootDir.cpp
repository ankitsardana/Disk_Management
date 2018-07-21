#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stack>
using namespace std;
char bufferSector[512];
char bufferIn[512];
stack <int> dir_flow;
stack <int> del_use;

class rootDir
{	char disk_name[1000];
	int disk_size;                         	 // file size
	public: int res_sector;                         //no of sectors that are reserved to store status of sectors.
	int total_sector;                       //Total number of sectors
	const int buffersize=512;               //size of one sector
	
	int array_freeSec[100000];
	public: int current_directory;
	struct sectorStatus;
	struct fileEntry;
	struct fileStatus;
	

	public:
		rootDir();
		void res_write();
		void Dir_write(int sector_no);
		void unres_write();
		void create_file(char filename[19], int size, char a);
		void create_directory(char filename[19]);
		void read_file(char filename[19]);
		void delete_file(char filename[19]);
		void restore_file(char filename[19]);
		void permanent_delete(char filename[19]);
		int check_status(int index);					// int index: sector number
		void update_status(int index, int temp);			// int index: sector number
		void write_sector(int index, char bufferSector[512]);		// int index: sector number  char a: character to be inserted
		void read_sector(int index);						// int index: sector number
		int sector_number(int index);					// int index: sector number
		int byte_number(int index);					// int index: sector number
		void display(int index);					// int index: sector number
		void free_sectors(int num);					// int num: number of free sectors
		void movein_directory(char filename[19]);
		void moveto_parent_directory();
		void currentdir_name();
		void delete_directory(char filename[19]);
		void restore_directory(char filename[19]);
		void permanent_delete_directory(char filename[19]);
		void format_disk();
                void list_directory();
                void txtfile_create_file(char filename1[30],char filename[19]);
		void read_file_writetxt_file(char filename1[30],char filename[19]);	                
};


typedef struct 
    {
		union{
			
			int num;
			char buffer[4];
		};
	     
    }sectorStatus;
sectorStatus d;					//declare object of sectorStatus structure globally.

struct fileEntry
    {
			char file_name[19];       // y if directory else n.
			char is_directory;
			int start_sector;
			int file_size;
			int parent_dir;
						
    };	

struct fileStatus
{
	union{
		fileEntry file;
		char filebuff[32]; };
}root;


rootDir::rootDir()
/*
    Objective:To create a new disk if disk of the given file name doesn't exist and write 0000 in reserved sector, 0000 in the first two unreserved sectors andA,B,C,...J in the remaining unreserved sector.
*/
{
	
	ifstream f;
	cout << "enter disk name: ";
	cin >> disk_name;
	f.open(disk_name, ios::binary | ios::in | ios::out);
	bool createNew = (f.peek()==ifstream::traits_type::eof());
	f.close();

	if(createNew){	
	    cout << "Enter disk size(any integer) in MB: ";
	    cin >> disk_size;
	    total_sector = (disk_size*pow(2,20))/pow(2,9);
	    res_sector = total_sector/pow(2,7);
	    res_write();
	    Dir_write(res_sector);
	    unres_write();
	    cout << "Disk " << disk_name << " has been created !!" <<endl;
	}
	else {
		int size;
		cout<<"Disk already exists!!"<<endl;
		ifstream fp;
		fp.open(disk_name,ios::in|ios::binary);
		fp.seekg(0,ios::end);
		size=fp.tellg();
		fp.close();
		disk_size=size/pow(2,20);
		total_sector=(disk_size*pow(2,20))/pow(2,9);
		res_sector=total_sector/pow(2,7);
	}
	
	
	    cout<<"total number of sectors in "<<disk_size<<" MB disk: "<<total_sector<<endl;
	   	cout<<"total number of reserved sectors in "<<disk_size<<" MB disk: "<<res_sector<<endl;

}

void rootDir::res_write()
 /*Objective:function to write 0000 in all the reserved sectors.
    Approach:using union
 */
{	fstream f;
	f.open(disk_name,ios::out|ios::binary);
	d.num=0;
	for(int i=0;i<total_sector;i++)
		f.write(d.buffer,4);
	f.close();
	return ;
}

void rootDir::Dir_write(int sector_no)
 /*Objective:function to write 0000 in the first two unreserved sectors.
    Approach:using union
 */
{	fstream f;
	f.open(disk_name,ios::out|ios::in|ios::binary);
	f.seekp((sector_no)*buffersize);
	d.num=0;
	for(int i=0;i<32;i++)
		f.write(d.buffer,32);
	f.close();
	return ;
}

void rootDir::unres_write()
/*
	Objective:Initialise the disk, store A, B,..., J in empty sectors.
*/
{
        fstream f;
	int a=65;                                 //65: ASCII value of character A
	f.open(disk_name,ios::in|ios::out|ios::binary);
	f.seekp((res_sector+2)*buffersize);
	for(int i=0;i<(total_sector-res_sector-2);i++)
	{	for(int j=0;j<buffersize;j++)
			bufferSector[j]=char(a+i%10);
		f.write(bufferSector,buffersize);
	}
	f.close();
	return;
}

void rootDir::create_file(char filename[19],int size, char a)
{	
	
	for(int k = 0; k < 2; k++)
	{
		read_sector(current_directory + k);
		for(int l = 0; l < 16; l++)
		{	for(int i = 0; i < 32; i++)
				root.filebuff[i] = bufferSector[32*l+i];
				
			if((strcmp(root.file.file_name, filename) == 0)&&(root.file.is_directory =='n'))
			{       
				cout<<"file exists with the same name !! so choose option 1 & give a different name !!"<<endl;
				return;
			}
		        else if(int(bufferSector[32*l]) == 0)
		       
			{	for(int i=0;i<19;i++)
			        	root.file.file_name[i]=filename[i];
				root.file.file_size = size;
				int noOfSector = (size - 1)/512 + 1;			//total number of sectors required to store file
				free_sectors( noOfSector);
				
				if( array_freeSec[0] != 0)					
				
				{	
					for(int i = 0; i < noOfSector; i++)
					{
						update_status((array_freeSec[i]), array_freeSec[i + 1]);
						for(int j = 0; j < buffersize; j++)
						{	
							if((512 * i + j) < root.file.file_size)
								bufferSector[j] = a;
							else
								bufferSector[j] = char(' ');
						}
						write_sector(array_freeSec[i] , bufferSector);
					}

					root.file.start_sector = array_freeSec[0];
					root.file.parent_dir = current_directory;
					root.file.is_directory = 'n';
					read_sector(current_directory + k);
					for(int m = 0; m < 32; m++)
						bufferSector[32*l + m] = root.filebuff[m];
					write_sector((current_directory + k) , bufferSector);
					cout << "file has been created !!" <<endl;
					return;

				}
				else
					{
						cout << "not enough free space in disk to store the file" << endl;
						return;
					}
				
			}
		}	
	}
	cout << "Not free space in directory" << endl;
	return;
}


void rootDir::read_file(char filename[19])
{
	for(int i = 0; i < 2; i++)
	{
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];
			if((strcmp(root.file.file_name, filename) == 0) && (root.file.is_directory=='n'))
			{
				if(check_status(root.file.start_sector) > 0)
				{
					int temp = root.file.start_sector;
					do
					{
						display(temp);
						temp = check_status(temp);
					}while(temp != 1);
					return;
				}
				else
					cout << "Deleted file!! To read deleted file first restore it!!" << endl;
					return;
			}
			read_sector(current_directory + i);
			
		}
	}
	cout << "file doesn't exist !!" << endl;
	return;
}


void rootDir::delete_file(char filename[19])
{
	for(int i = 0; i < 2; i++)
	{
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];
			if(strcmp(root.file.file_name, filename) == 0 )
			{	
				if(check_status(root.file.start_sector) > 0)
				{	
					int temp = root.file.start_sector;
					int num = check_status(temp);
					do
					{	
					
						update_status(temp, (-1)*num);
						temp = (-1)*check_status(temp);
						num = check_status(temp);
					}while(temp != 1);
					cout << "File has been deleted!!" << endl;
					return;
				}
				read_sector(current_directory + i);
				
			}
		}
	}
	cout << "file doesn't exist !!"<<endl;
	return;
}

void rootDir::restore_file(char filename[19])
{
	for(int i = 0; i < 2; i++)
	{
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];
			if(strcmp(root.file.file_name, filename) == 0 )
			{	
				if(check_status(root.file.start_sector) <  0)
				{	
					int temp = root.file.start_sector;
					int num = check_status(temp);
					do
					{	
					
						update_status(temp, (-1)*num);
						temp = check_status(temp);
						num = check_status(temp);
					}while(temp != 1);
					cout << "File has been restored!!" << endl;
					return;
				}
				else
					cout << "File exist in the directory!!" << endl;
					return;
				read_sector(current_directory + i);
			}
		}
	}
	cout << "file doesn't exist !!"<<endl;
	return;
}


void rootDir::permanent_delete(char filename[19])
{       
        int temp1;
	for(int i = 0; i < 2; i++)
	{
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];
			if(strcmp(root.file.file_name, filename) == 0 )
			{
				int temp = root.file.start_sector;
				do
				{	temp1 = check_status(temp);
					update_status(temp, 0);
					temp = temp1;
				}while(temp != 1);
				cout << "File has been completely deleted" << endl;
				return;
			}	
			read_sector(current_directory + i);
			
		}
	}
	cout << "file doesn't exist !!"<<endl;
	return;
}



void rootDir::free_sectors(int num)
/*
	Objective: to store sector number of free sectors in array array_freeSec;
	input parameter:
		num: number of free sectors
*/
{
	int j = 0;
	for(int i = 0; i < (total_sector-res_sector-2); i++)
	{	
		if(check_status(res_sector + 2 + i)==0)
		{	
			array_freeSec[j] = res_sector + 2 + i;
			j++;
		}
		if(j == num)
		{
			array_freeSec[num] = 1;
			return;
		}
	}
	array_freeSec[0] = 0;
		return;
}

int rootDir::check_status(int index)
/*
    Objective:to check wheather a sector is reserved, root directory, free or busy.
    Approach: check status using union.
    Input Parameter:
        index: Sector number
    Return value:
    	return 2: if sector is reserved sector
    	return 3: if root directory
        else:
        	return content of the status
     
*/
{   if(0<=index && index<res_sector)
		return 2;
	else if (index < res_sector+2)
		return 3;
	else
	{	
        fstream f;
        int sector_pos , byte_pos, temp;
        sector_pos = sector_number(index);
        byte_pos = byte_number(index);
	    f.open(disk_name,ios::in|ios::binary);
	    f.seekg(sector_pos*buffersize);
	    f.read(bufferSector,buffersize);
	    f.close();
	    for(int i=0;i<4;i++)
			d.buffer[i]=bufferSector[byte_pos+i];
    	    return d.num;
	}
}

int rootDir::sector_number(int index)
/*
	Objective: to get sector number where the status of the given sector stored in reserved sector.
	input paramete: 
		indx: sector number
	Return Value:
		sector number where the status stored
*/
{
	int num;
	num = (index-res_sector)/128;
	return num;
}

int rootDir::byte_number(int index)
/*
	Objective: to get byte number where the status of the given sector stored in reserved sector.
	input paramete: 
		indx: sector number
	Return Value:
		byte number where the status is stored
*/
{
	int num;
	num=((index-res_sector)%128)*4;
	return num;
}

void rootDir::write_sector(int index , char bufferSector[512])
/*
    Objective: to write in a free sector byte by byte
    Input parameter:
        index:sector number
        a: character pointer to an array that is to be write in the sector.
*/
{   fstream f;
	f.open(disk_name,ios::out|ios::in|ios::binary);
	
	f.seekp((index)*buffersize);
	f.write(bufferSector,buffersize);
	
	f.close();
	return;
}

void rootDir::update_status(int index, int num)
/*
	Objective: To update status of a sector to given integer num
	Return value:
		index: enter sector number
		num: value that is to be write in the status
*/
{
	fstream f;
	d.num = num;
	f.open(disk_name,ios::in|ios::out|ios::binary);
	f.seekg((index-res_sector)/128*buffersize);
	f.read(bufferSector,buffersize);
	for(int i=0;i<4;i++)
	        {
		bufferSector[(((index-res_sector)%128)*4+i)]=d.buffer[i];}
	f.seekp((index-res_sector)/128*buffersize);
	f.write(bufferSector,buffersize);
	f.close();
	return;
}


void rootDir::read_sector(int index)				//function read whose return type is character pointer.
/*
    Objective: to read the content of a sector byte by byte
    Input parameter:
        index:sector number
*/
{   fstream f;
	f.open(disk_name,ios::in|ios::binary);
	f.seekg((index)*buffersize);
	f.read(bufferSector,buffersize);
	f.close();
	return;
}

void rootDir::display(int index)
/*
    Objective: to read the content of a sector byte by byte
    Input parameter:
        index:sector number
*/
{
	
	int temp=check_status(index);
	read_sector(index);
	if(temp==2)
	{
		for(int i=0; i < buffersize; i++)
			cout << int(bufferSector[i]);
		cout << endl;
	}
	else 
	{
		for(int i = 0; i < buffersize; i++)
			cout << bufferSector[i];
		cout << endl;
	}
	return;
}




void rootDir::create_directory(char filename[19])
{	
	
	for(int k = 0; k < 2; k++)
	{
		read_sector(current_directory + k);
		for(int l = 0; l < 16; l++)
		{	
		        for(int i = 0; i < 32; i++)
				root.filebuff[i] = bufferSector[32*l+i];
		        if((strcmp(root.file.file_name, filename) == 0)&&(root.file.is_directory =='y'))
			{       
			        
				cout<<"DIRECTORY exists with the same name !! so choose option 7 & give a different name !!"<<endl;
					
			        return;
			}
			else if(int(bufferSector[32*l]) == 0)
			{	
			        strcpy(root.file.file_name,filename);
				root.file.file_size = 1024;
				do{
					free_sectors(2);
					}
				while((array_freeSec[0]+1)!=array_freeSec[1]);
				if( array_freeSec[0] != 0)					
				
				{	
					for(int i = 0; i < 2; i++)
						update_status((array_freeSec[i]), 4);
					Dir_write(array_freeSec[0]);	
					root.file.start_sector = array_freeSec[0];
					root.file.parent_dir = current_directory;
					root.file.is_directory = 'y';
					read_sector(current_directory + k);

					for(int m = 0; m < 32; m++)
						bufferSector[32*l + m] = root.filebuff[m];
					write_sector((current_directory + k) , bufferSector);
					cout << "directory has been created !! as :" <<endl;
					return;

				}
				else
					{
						cout << "not enough free space in disk to make new directory" << endl;
						return;
					}
				
			}
		}	
	}
	cout << "Not free space in directory" << endl;
	return;
}





void rootDir::movein_directory(char filename[19])
{
	for(int i = 0; i < 2; i++)
	{       
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{       
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];	
			if(strcmp(root.file.file_name, filename) == 0)
			{
				if(check_status(root.file.start_sector) == 4)
				{
					dir_flow.push(current_directory);
					current_directory=root.file.start_sector;
					return;
				}
				else
					{
					cout << "Deleted directory!! To read deleted directory first restore it!!" << endl;
					return;}
			}
			read_sector(current_directory + i);
			
		}
	}
	cout << "no such directory exist !!" << endl;
	return;
}




void rootDir::moveto_parent_directory()				//function read whose return type is character pointer.
/*
    Objective: to read the content of a sector byte by byte
    Input parameter:
        index:sector number
*/
{   
string old_dirname;
if(dir_flow.empty()==true)
{
	cout<<"u r already in root directory"<<endl;
}
else{
	current_directory=dir_flow.top();
	dir_flow.pop();
}
cout<<"now you moved to parent directory of : "<<old_dirname<<" to :"<<root.file.file_name<<endl;
return;
}





void rootDir::list_directory()				//function read whose return type is character pointer.
/*
    Objective: to read the content of a sector byte by byte
    Input parameter:
        index:sector number
*/
{   
int sno,f;sno=1;f=0;
cout<<"\n\n\n======================================================================================\n";
cout<<"list of all the files and directories are as follows:"<<endl;
cout<<"s.no      file or directory name      Is Directory      start_sector      ";
for(int i = 0; i < 2; i++)
	{
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{       
		if(int(bufferSector[32*j]) == 0)
			{
				return;
			}
		else    {	
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];
			cout<<" "<<sno<<"        "<<root.file.file_name<<"             "<<root.file.is_directory<<"                "<<root.file.start_sector<<endl;
			sno++;	
		        }		
		}
		

	}
	return;
}







void rootDir::delete_directory(char filename[19])
{
	for(int i = 0; i < 2; i++)
	{
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];
			if(strcmp(root.file.file_name, filename) == 0 )
			{	
				if(check_status(root.file.start_sector) == 4)
				{	
					for(int l=0;l<2;l++)
					{
						update_status((root.file.start_sector+l), -4);
					}
					cout << "Directory has been deleted!!" << endl;
					return;
				}
				read_sector(current_directory + i);
				
			}
		}
	}
	cout << "directory doesn't exist !!"<<endl;
	return;
}

void rootDir::restore_directory(char filename[19])
{
	for(int i = 0; i < 2; i++)
	{
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];
			if(strcmp(root.file.file_name, filename) == 0 )
			{	
				if(check_status(root.file.start_sector) == -4)
				{	
					for(int l=0;l<2;l++)
					{
						update_status((root.file.start_sector+l), -4);
					}
					cout << "Directory has been restored!!" << endl;
					return;
				}
				else
					cout << "directory exist in the directory!!" << endl;
					return;
				read_sector(current_directory + i);
			}
		}
	}
	cout << "directory doesn't exist !!"<<endl;
	return;
}


void rootDir::permanent_delete_directory(char filename[19])
{       
        
	for(int i = 0; i < 2; i++)
	{
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];
			if(strcmp(root.file.file_name, filename) == 0 )

			{       
				
	                        movein_directory(filename);	
	                        del_use.push(current_directory);	
			        for(int l = 0; l < 2; l++)
					{
						read_sector(current_directory + l);
						for(int m = 0; m < 16; m++)
							{	
								if(int(bufferSector[32*m]) != 0)
									{
										for(int n = 0; n < 32; n++)
											root.filebuff[n] = bufferSector[32*m+n];
										if(root.file.is_directory=='n')
											permanent_delete(root.file.file_name);
										else if(root.file.is_directory=='y')
											permanent_delete_directory(root.file.file_name);	
												
									}
								else
									{
										moveto_parent_directory();
										for(int g=0;g<2;g++)
										{
											update_status((del_use.top()+g), 0);
											
										}
										del_use.pop();
										if(del_use.empty())
										{	
											cout<<"directory has been deleted permanently"<<endl;
											return;
										}
										else
											return;
									}
									read_sector(current_directory + l);
							  }
					}
			
				
				
			}	
			
		read_sector(current_directory + i);	
		}
	}
	cout << "file doesn't exist !!"<<endl;
	return;
}





void rootDir::format_disk()				//function read whose return type is character pointer.
/*
    Objective: to read the content of a sector byte by byte
    Input parameter:
        index:sector number
*/
{   	    
            res_write();
	    Dir_write(res_sector);
	    unres_write();
	    cout << "Disk " << disk_name << " has been formated !!" <<endl;

return;
}





void rootDir::currentdir_name()				//function read whose return type is character pointer.
/*
    Objective: to read the content of a sector byte by byte
    Input parameter:
        index:sector number
*/
{     
      int temp;
      temp=current_directory;
      current_directory=dir_flow.top();
      dir_flow.push(temp);		    
      for(int i = 0; i < 2; i++)
	{
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];
			if(int(root.file.start_sector)==temp) 
			{
				
					cout << "name of current directory is :" <<root.file.file_name<< endl;
					current_directory=dir_flow.top();
                                        dir_flow.pop();
					return;
			}
			
			
		}
	}
	
}





void rootDir::txtfile_create_file(char filename1[30],char filename[19])
{	

  int size;
  char ch;		
		ifstream fp1;
		fp1.open(filename1,ios::in|ios::binary);
		fp1.seekg(0,ios::end);
		size=fp1.tellg();
		fp1.seekg(0,ios::beg);
		  
	
	for(int k = 0; k < 2; k++)
	{
		read_sector(current_directory + k);
		for(int l = 0; l < 16; l++)
		{	for(int i = 0; i < 32; i++)
				root.filebuff[i] = bufferSector[32*l+i];
				
			if((strcmp(root.file.file_name, filename) == 0)&&(root.file.is_directory =='n'))
			{       
				cout<<"file exists with the same name & the data is overwritten in that:"<<endl;
				if(size<=root.file.file_size)
				{	
					int sector_no=root.file.start_sector;
					int i=0;
					char ch;
					do
					{
					for(int j = 0; j < buffersize; j++)
						{	
							if((512 * i + j) < root.file.file_size)
							{	
								fp1.get(ch);
								bufferSector[j] = ch;
							}
							else
								bufferSector[j] = char(' ');
						}
						write_sector(sector_no , bufferSector);
						i++;
						sector_no=check_status(sector_no);
					}while(sector_no!=1);
						
				}
				else
				{
					int diff_size=size-root.file.file_size;
					int noOfSector=(diff_size-1)/512+1;
					int last_sector;
					free_sectors(noOfSector);
					if( array_freeSec[0] != 0)
					{
						int sector_no=root.file.start_sector;
						int i=0;
						do
						{
						for(int j = 0; j < buffersize; j++)
							{	
								if((512 * i + j) < root.file.file_size)
								{	
									fp1.get(ch);
									bufferSector[j] = ch;
								}
								else
									bufferSector[j] = char(' ');
							}
							write_sector(sector_no , bufferSector);
							i++;
							last_sector=sector_no;
							sector_no=check_status(sector_no);
						}while(sector_no!=1);		
						update_status(last_sector,array_freeSec[0]);
						for(int i = 0; i < noOfSector; i++)
						{
							update_status((array_freeSec[i]), array_freeSec[i + 1]);
							for(int j = 0; j < buffersize; j++)
							{	
								if((512 * i + j) < root.file.file_size)
								{	
									fp1.get(ch);
									bufferSector[j] = ch;
								}
								else
									bufferSector[j] = char(' ');
							}
							write_sector(array_freeSec[i] , bufferSector);
						}
						
						
					}
				
				}
				fp1.close();
				return;
			}
		        else if(int(bufferSector[32*l]) == 0)
		       
			{	for(int i=0;i<19;i++)
			        	root.file.file_name[i]=filename[i];
				root.file.file_size = size;
				int noOfSector = (size - 1)/512 + 1;			//total number of sectors required to store file
				free_sectors( noOfSector);
				
				if( array_freeSec[0] != 0)					
				
				{	
					for(int i = 0; i < noOfSector; i++)
					{
						update_status((array_freeSec[i]), array_freeSec[i + 1]);
						for(int j = 0; j < buffersize; j++)
						{	
							if((512 * i + j) < root.file.file_size)
								{	
									fp1.get(ch);
									bufferSector[j] = ch;
								}
							else
								bufferSector[j] = char(' ');
						}
						write_sector(array_freeSec[i] , bufferSector);
					}

					root.file.start_sector = array_freeSec[0];
					root.file.parent_dir = current_directory;
					root.file.is_directory = 'n';
					read_sector(current_directory + k);
					for(int m = 0; m < 32; m++)
						bufferSector[32*l + m] = root.filebuff[m];
					write_sector((current_directory + k) , bufferSector);
					cout << "file has been created !!" <<endl;
					fp1.close();
					return;

				}
				else
					{
						cout << "not enough free space in disk to store the file" << endl;
						fp1.close();
						return;
					}
				
			}
		}	
	}
	cout << "Not free space in directory" << endl;
	fp1.close();
	return;
}





void rootDir::read_file_writetxt_file(char filename1[30],char filename[19])
{       
	
	ofstream fp1;
		fp1.open(filename1,ios::in|ios::binary);
		fp1.seekp(0,ios::beg);
	
	for(int i = 0; i < 2; i++)
	{
		read_sector(current_directory + i);
		for(int j = 0; j < 16; j++)
		{
			for(int k = 0; k < 32; k++)
				root.filebuff[k] = bufferSector[32*j+k];
			if((strcmp(root.file.file_name, filename) == 0) && (root.file.is_directory=='n'))
			{
				if(check_status(root.file.start_sector) > 0)
				{
					int temp = root.file.start_sector;
					do
					{       
					
						
						read_sector(temp);
						
						{
							for(int i = 0; i < buffersize; i++)
								fp1 << bufferSector[i];
							cout << endl;
						}
	
					
					
						
						temp = check_status(temp);
					}while(temp != 1);
					fp1.close();
					return;
				}
				else
					cout << "Deleted file!! To read deleted file first restore it!!" << endl;
					fp1.close();
					return;
			}
			read_sector(current_directory + i);
			
		}
	}
	cout << "file doesn't exist !!" << endl;
	fp1.close();
	return;
}






int main()
{   rootDir mydirectory;
    int x, index,filen;
    char filename[19];
    char filename1[30];
    mydirectory.current_directory=0;
    while(1)
	{
	if(mydirectory.current_directory==0)
	{
	mydirectory.current_directory=mydirectory.res_sector;
	}
	cout << "------------*-----------------MENU-----------------*--------------------"<<endl;
	cout << "1:to create new file\n2:to check status of a sector\n3:to show the content of a sector\n4:read file\n5:Delete file(temprorily)\n6:Restore file\n7:create a new directory\n8:read stsrt adress of current directory\n9:to move to succesive directory\n10:to move to parent dierectory\n11:to list all the directories & files of the current directory \n12:to delete a directory (temprorily)\n13:delete a file(permanently)\n14:delete a directory (permanently)\n15:restore a directory\n16:to format disk \n17:to read a txt file & write its data in the file in the disk\n18:to read a file from the disk & write its data on to  a txt file \n19:to know the name of curent directory\n20:exit"<<endl;
	cin >> x;
	switch(x)
	{
	    case 1:
	    	{int size;
	    	char a;
	        cout << "enter file name(not more than 19 characters): ";
		    cin >> filename;
		    cout << "enter file size(in bytes):" << endl;
		    cin >> size;
		    cout << "enter character that is to be write in the file: " << endl;
		    cin >> a;
		    mydirectory.create_file(filename, size, a);
		    break;}
	    
	    case 2:
	        cout << "enter sector number to check it's status: ";
	        cin >> index;
	        if(mydirectory.check_status(index)==0)
	            cout << "sector is free" << endl;
	        else if(mydirectory.check_status(index) == 2)
	        	cout << "sector is reserved" << endl;
	        else if(mydirectory.check_status(index) == 3)
	        	cout << "root directory" << endl;
	        else if(mydirectory.check_status(index) == 4)
	        	cout << "child directory" << endl;	
	        else if(mydirectory.check_status(index) >= 1)
	        	cout << "sector is busy" << endl;
	        else 
	        	cout << "sector is deleted" << endl;
	        break;

	    case 3:
	    	cout << "enter sector number to display it's content: ";
	    	cin >> index;
	    	mydirectory.display(index);
	    	break;

	    case 4:
	    	cout << "enter file name: ";
	    	cin >> filename;
	    	mydirectory.read_file(filename);
	    	break;
	    case 5:
	    	cout << "enter file name: ";
	    	cin >> filename;
	    	mydirectory.delete_file(filename);
	    	break;
	    case 6:
	    	cout << "enter file name: ";
	    	cin >> filename;
	    	mydirectory.restore_file(filename);
	    	break;
	    case 7:
	    	{
	            cout << "enter directory name(not more than 19 characters): ";
		    cin >> filename;
		    mydirectory.create_directory(filename);
		    break;}	
	    case 8:
	    	cout << "adress of current directory is : "<<mydirectory.current_directory<<endl;
	    
	    	break;
	    case 9:	
	        cout <<"enter the name of directory u wanna move in:";
	        cin >>filename;
	        mydirectory.movein_directory(filename);
	        break;  
	    case 10:
	         
	    	mydirectory.moveto_parent_directory();
	    	break;
	    case 11:
	         
	    	mydirectory.list_directory();
	    	break; 
	    case 12:
	    	cout << "enter directory name: ";
	    	cin >> filename;
	    	mydirectory.delete_directory(filename);
	    	break; 
	    case 13:
	    	cout << "enter file name: ";
	    	cin >> filename;
	    	mydirectory.permanent_delete(filename);
	    	break;
	    case 14:
	    	cout << "enter directory name: ";
	    	cin >> filename;
	    	mydirectory.permanent_delete_directory(filename);
	    	break;
	    case 15:
	    	cout << "enter directory name: ";
	    	cin >> filename;
	    	mydirectory.restore_directory(filename);
	    	break;
	    case 16:
	    	mydirectory.format_disk();
	    	break;
	    case 17:
	    	cout << "enter file name whose data to be entered: ";
	    	cin >> filename1;
	    	cout << "enter file name in which data to be entered: ";
	    	cin >> filename;
	    	mydirectory.txtfile_create_file(filename1,filename);
	    	break;
	    case 18:
	    	cout << "enter txt file name in which data to be entered: ";
	    	cin >> filename1;
	    	cout << "enter disk file name from which data to be entered: ";
	    	cin >> filename;
	    	mydirectory.read_file_writetxt_file(filename1,filename);
	    	break;
	    case 19:
	    	mydirectory.currentdir_name();
	    	break;	    	
	 	case 20:return 0;
	    default:cout << "wrong choice" << endl;
	}
	}
}
