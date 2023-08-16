namespace LegoTrainManager.Models;

/// <summary>
/// the electrical motor in a train
/// </summary>
public class TrainEngine : Powerconsumer
{
    public bool Forward { get; set; }


    public TrainEngine()
    {
        ConsumerType = ConsumerType.Engine;
    }
}