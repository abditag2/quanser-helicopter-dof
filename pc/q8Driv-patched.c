#include "q8Driv.h"
#include "q8Driv-betti.h"
#define DAC_CH_MASK 0x03
#define ENC_CH_MASK 0x07
#define DAC_CH_SIZE 2
#define ENC_CH_SIZE 3
#define ENC_FLGS  3
#define DAC_RNG 10
#define BIPOLAR 1

MODULE_AUTHOR("Quanser Inc.");
MODULE_ALIAS("Q8driver");
MODULE_DESCRIPTION("Q8 Driver");
MODULE_LICENSE("Proprietary");
/*
int Q8_Initialise (int BoardNum);
int Q8_EncInitialise(int BoardNum, unsigned char Chan, char BCD, unsigned char Mode, 
					 unsigned char Quadr, char IndexEn, char IndexPolarity);
void Q8_EncSetFilterPrescaler (int BoardNum, int nChannel, int nValue);
void Q8_EncConfigureIO (int BoardNum, int nChannel, int nFlags, int bLoadCounterOnIndex);
int Q8_DACPreConfigure (int BoardNum, unsigned char nChannel, char bBipolar, int nRange);
unsigned short int Q8_DACVoltageToOutput(double nVoltage, unsigned char bBipolar, double nRange );
int Q8_SetDAC(int BoardNum, char ChanMask, short int *v);
*/
//static irqreturn_t q8_interrupt(int irq, void *data, struct pt_regs *regs)
static irqreturn_t q8_interrupt(int irq, void *data)
{
	return IRQ_NONE;
}

int  Q8_Probe (struct pci_dev *dev, const struct pci_device_id *id)
{
	unsigned long mem_start, mem_len, mem_flags;
	PQ8Registers base_addr = NULL;
	int err = 0;
	PQ8 priv;

	priv = kmalloc(sizeof(Q8),GFP_KERNEL);
	if (!priv)
	{
		printk(KERN_ALERT "no memory\n");
		err = -ENOMEM;
		return err;
	}

	mem_start = pci_resource_start(dev, 0);
	mem_len = pci_resource_len(dev, 0);
	mem_flags = pci_resource_flags(dev, 0);

	if ((mem_flags & IORESOURCE_MEM) != IORESOURCE_MEM) {
		printk(KERN_ALERT "weird - resource type is not memory\n");
		err = -ENODEV;
		kfree(priv);
		return err;
	}

	base_addr = ioremap_nocache(mem_start, mem_len);
	if (!base_addr)
	{
		printk(KERN_ALERT "Error calling ioremap_nocache.\n");
		err = -EIO;
		kfree(priv);
		return err;
	}

	priv->m_pRegisters = base_addr;
	err = pci_enable_device(dev);

	if (err)
	{
		printk(KERN_ALERT "enable device error %d\n",err);
		iounmap(base_addr);
		kfree(priv);
		return err;
	}

	err = request_irq(dev->irq, q8_interrupt, IRQF_SHARED, DRV_NAME, priv);//SA_SHIRQ, DRV_NAME, priv); 
	if (err)
	{
		printk(KERN_ALERT
		       "Error calling request_irq: %d.\n", dev->irq);
		pci_disable_device(dev);
		iounmap(base_addr);
		kfree(priv);
		return err;
	}
	pci_set_drvdata(dev, priv);

	Boards.BrdID[Boards.Count] = priv;
	Boards.BrdInit[Boards.Count] = 1;
	Boards.Count++;
	return 0;
}

void Q8_Remove (struct pci_dev *dev)
{
	PQ8 priv = pci_get_drvdata(dev);

	if (priv)
	{
		if (dev->irq)
			free_irq(dev->irq, priv);

		if (priv->m_pRegisters)
			iounmap((void __iomem *)priv->m_pRegisters);
	}
	pci_disable_device(dev);

	kfree(priv);
	return;
}

#ifdef CONFIG_PM
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
static int Q8_suspend(struct pci_dev *pci_dev, u32 state)
#else
static int Q8_suspend(struct pci_dev *pci_dev, pm_message_t state)
#endif
{
	printk(KERN_ALERT "Suspend Function called\n");
	return 0;
}

static int Q8_resume(struct pci_dev *pci_dev)
{
	printk(KERN_ALERT "Resume Function called\n");
	return 0;
}
#endif

static struct pci_driver Q8_driver = {
	.name = DRV_NAME,
	.id_table = Q8_devIDs,
	.probe = Q8_Probe,
	.remove = Q8_Remove,
#ifdef CONFIG_PM
	.suspend = Q8_suspend,
	.resume = Q8_resume,
#endif
};

MODULE_DEVICE_TABLE(pci, Q8_devIDs);
int __init Q4Init (void)		
{
	int err;
	int i;
	short int val[DAC_CH_SIZE];
	uint8_T temp = ENC_FLGS;
	err = Q8_Initialise(0);
	if (err == -1)
	{
		printk(KERN_ALERT "Q4 could not be Initialized, board not present");
		return -EIO;
	}
	for (i = 0; i < ENC_CH_SIZE; i++)
	{
		err = Q8_EncInitialise(0, i, 0, 0, 3, 1, 1);
		if (err == -1)
		{
			printk(KERN_ALERT "Q4 ENC %d could not be Initialized", i);
			return -1;
		}
	}
	for (i = 0; i < ENC_CH_SIZE; i++)
	{
		Q8_EncSetFilterPrescaler(0, i, 0);
	}
	for (i = 0; i < ENC_CH_SIZE; i++)
	{
		Q8_EncConfigureIO(0, i, temp, 0);
	}
	for (i = 0; i < ENC_CH_SIZE; i++)
	{
		err = Q8_DACConfigure(0, i, BIPOLAR, DAC_RNG);
		if (err == -1)
		{
			printk(KERN_ALERT "Q4 DAC %d could not be Initialized", i);
			return -1;
		}
	}
	for (i = 0; i < DAC_CH_SIZE; i++)
	{
		val[i] = Q8_DACVoltageToOutput(0, BIPOLAR, DAC_RNG);
	}
	/*err =*/ Q8_SetDAC(0, DAC_CH_MASK, val);
	/*if (err != 0)
	{
		printk(KERN_ALERT "Q4 DAC's could not be set to Zero for open\n"); 	
		return -1;
	}*/
	printk(KERN_ALERT "Q4 Has been Initialized\n");
	return 0;
}

static int Q8_init(void)
{
	int err;
	int i;
	
	for(i = 0; i < 8; i++)
		Boards.BrdInit[i] = 0;
	
	Boards.Count = 0;

	err = pci_register_driver(&Q8_driver); //pci_module_init(&Q8_driver);

	if (err)
	{
        printk(KERN_ALERT "Q8 Driver Load Error\n");
		return err;
	}

    printk(KERN_ALERT "Q8 Driver create char device...\n");

	chardev_init();
	err = Q4Init();
	if (err != 0)
	{
		printk("If your in here Q4Init better have failed");
		chardev_clean();
		pci_unregister_driver(&Q8_driver);
		return -EIO;
	}
// This has been moved to seperate Module Function Q4INIT

 //   	err = Q8_Initialise(0);					// initialise Q8 Board 0
//	Q8_DACConfigure(0, 0, 1, 10);	// configure the DAC: brd 0, channel 4, bipolar, 10V range
//	Q8_DACConfigure(0, 1, 1, 10);
	//Q8_DACConfigure(0, 2, 1, 10);  not testing all Dac's anymore
	//Q8_DACConfigure(0, 3, 1, 10);
//	Q8_EncInitialise(0,0,0,0,3,1,1);	// configure the encoder: brd 0, encoder 0, four quad, index & polarity set
//	Q8_EncInitialise(0,1,0,0,3,1,1);
//	Q8_EncInitialise(0,2,0,0,3,1,1);
/*	if (err == -1)
	{
		printk(KERN_ALERT "WTF %d \n", err);
		chardev_clean();
		pci_unregister_driver(&Q8_driver);
		return err;
	}*/
	printk("Q8 Test Module Running:\n"); //printk(KERN_ALERT "Q8 Test Module Running:\n");
    printk(KERN_ALERT "Q8 Driver Loaded, boards: %d\n", Boards.Count);
	return err;
}
void __exit Q4Exit (void)
{
	//int err;
	short int val[DAC_CH_SIZE];
	int i;
	for (i = 0; i < DAC_CH_SIZE; i++)
	{
		val[i] = Q8_DACVoltageToOutput(0, BIPOLAR, DAC_RNG);
	}
	/*err = */Q8_SetDAC(0, DAC_CH_MASK, val);
	/*if (err != 0)
	{
		printk(KERN_ALERT "Q4 DAC's could not be set to Zero for close\n"); 
		return err;
	}*/
	printk(KERN_ALERT "Q4 has been Uninitialized.\n");
}

static void __exit Q8_exit(void)
{
	//int err;
	/*err = */Q4Exit();
	/*if (err != 0)
		printk(KERN_ALERT "Q4 could not be Unitialized");*/
	chardev_clean();
	pci_unregister_driver(&Q8_driver);
	printk(KERN_ALERT "Q8 Driver Removed\n");
}

module_init(Q8_init);
module_exit(Q8_exit);


