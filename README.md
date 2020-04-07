# Embedded node protocol library
General purpose this library is create user interface from Embedded aplication.  
You create nodes in your application and Embedded Node Configurator will be able to show it for usergit.  
The structure of nodes is completely formed in your project.  
You can download the Configurator [here](https://github.com/embnode/ENConfigurator/releases/).  
The lastest library version you can find on [release page](https://github.com/embnode/enp/releases).  

# Embedded node protocol  
It is application layer protocol according OSI model. It use master - slave architecture.  
Master is Configurator. Slave is a device in the network. Access to devices can be done by ID.  
If you want to know more visit the [wiki](https://github.com/embnode/enp/wiki) library.

### Phisical layer
It can use serial interfaces such as RS232, RS485, RS422, UART, USB serial.   
If you want use CAN interface you should use embedded node CAN protocol.  
If you have free interface you can implement all features of this library.  
All what you need to do two functions - the read and the write.  

## Storing
The library have storing interface. If you want to write nodes values in non-volatile memory,  
you will have to implement own flash write and erase function. This functions have to use flash interface  
of the library.

## Contributing
If you have found a bug or have idea then write to the [issue page](https://github.com/embnode/enp/issues).
