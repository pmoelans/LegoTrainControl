using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;
using LegoTrainManager.Controllers;
using LegoTrainManager.Data;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.Metadata.Internal;
using MQTTnet;
using MQTTnet.Server;

namespace LegoTrainManager.Models
{

    public enum ConsumerType
    {
        Light=1,
        Engine=2
    }

    public enum DeviceType
    {
        Train=1,
        Switch=2
    }

    public class Powerconsumer
    {
        [Key,DatabaseGenerated(DatabaseGeneratedOption.Identity)]
        public int Id { get; set; }

        public string Name { get; set; }
        public int PowerPercentage { get; set; }

        public ConsumerType ConsumerType { get; set; }

    }

    
   

    /// <summary>
    /// abstract class that describes the deived that is powered by a battery
    /// </summary>
    public class BatteryPoweredDevice
    {
        [Key, DatabaseGenerated(DatabaseGeneratedOption.Identity)]
        public int Id { get; set; }

        public string Name { get; set; }

        public double BatteryVoltage { get; set; }

        public DateTime LastConnectionTime { get; set; }

        public bool Online { get; set; }

        public DeviceType DeviceType { get; set; } 


    }

    /// <summary>
    /// This class represents the data that we are getting from the MQTT broker consering the trains
    /// </summary>
    public class Train: BatteryPoweredDevice
    {
        public Train()
        {
            DeviceType = DeviceType.Train;
        }

        public ICollection<Powerconsumer> Powerconsumers { get; set; }

    }

    public interface IMqttMessageHandler
    {
        public Task HandleMqttMessage(MqttApplicationMessage message);
        public string Topic2Subscribe2 { get; }
    }

    public interface IMQTTTrainControlHandler: IHostedService
    {
        Task PublishAsync(string topic, int tractionPercentage);

    }

    public class StatusHandler: IMqttMessageHandler
    {
        public ITrainSearchEngine TrainSearchEngine { get; }

        public StatusHandler() : this(new TrainSearchEngine(ApplicationDbContext DbContext))
        {

        }
        private StatusHandler(ITrainSearchEngine trainSearchEngine)
        {
            TrainSearchEngine = trainSearchEngine;
        }
        public Task HandleMqttMessage(MqttApplicationMessage message)
        {

           // var msg = message.PayloadSegment;
            var str= message.ConvertPayloadToString();
            
            var t = new Task(() => { Console.WriteLine(message.PayloadSegment); });
            t.Start();
            return t;
        }

        public string Topic2Subscribe2 { get; } = "Lego/Trains/Status/#";
    }
}
