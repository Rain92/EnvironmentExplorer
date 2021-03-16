#define POWERPIN GPIO_NUM_33
#define BATTERYSENSEPIN GPIO_NUM_32
#define BATTERYPULLDOWNPIN GPIO_NUM_26
#define VOLTAGEDIVIDERFACTOR 2.07
#define VOLTAGECORRECTIONFACTOR 0.9575

void InitBatteryMonitor()
{
    pinMode(BATTERYPULLDOWNPIN, OUTPUT);
    digitalWrite(BATTERYPULLDOWNPIN, LOW);
    pinMode(BATTERYSENSEPIN, INPUT);
}

double ReadVoltage(byte pin)
{
    double reading = analogRead(pin);
    if (reading < 1 || reading > 4095)
        return 0;
    return -0.000000000000016 * pow(reading, 4) + 0.000000000118171 * pow(reading, 3) -
           0.000000301211691 * pow(reading, 2) + 0.001109019271794 * reading + 0.034143524634089;
}

double ReadVoltage(byte pin, int samples)
{
    double reading = 0;
    for (int i = 0; i < samples; i++)
    {
        reading += ReadVoltage(pin);
        delay(5);
    }
    return reading / samples;
}

double GetBatteryVoltage()
{
    return ReadVoltage(BATTERYSENSEPIN, 20) * VOLTAGEDIVIDERFACTOR * VOLTAGECORRECTIONFACTOR;
}
