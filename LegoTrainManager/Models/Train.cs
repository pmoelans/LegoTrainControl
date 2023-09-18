using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;
using System.Text.Json;
using LegoTrainManager.Controllers;
using LegoTrainManager.Data;
using Microsoft.EntityFrameworkCore;
using MQTTnet;


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
        public bool Reverse { get; set; }
       

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

        public bool Invert { get; set; }

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
        Task EstopTrain(Train train);
        Task ShutDownTrain(Train train);
        
        Task UpdateTrainControl(Train train, int speed);
        void AddMessageHandler(IMqttMessageHandler messageHandler);
    }

    public interface ITrainSearchEngine
    {
        Task UpdateTrainStatusAsync(TrainStatusClass trainStatus);
    }
    public class TrainSearchEngine:ITrainSearchEngine
    {
        private ApplicationDbContext _context;
        

        public TrainSearchEngine(ApplicationDbContext context)
        {
            _context = context;
           
        }

        public async Task UpdateTrainStatusAsync(TrainStatusClass trainStatus)
        {
           
        }

       
    }
    public class StatusHandler : IMqttMessageHandler
    {
        private readonly TrainsController _controller;




        public StatusHandler(TrainsController controller)
        {
            _controller = controller;

        }

        public async Task HandleMqttMessage(MqttApplicationMessage message)
        {

            // var msg = message.PayloadSegment;
            string str = message.ConvertPayloadToString();
            if (string.IsNullOrEmpty(str))
                return;
            var obj= JsonSerializer.Deserialize<TrainStatusClass>(str);

            UpdatedState?.Invoke(this,obj);
            await _controller.UpdateTrainState(obj);

        }


        public string Topic2Subscribe2 { get; } = "Lego/Trains/Status/LaboTrain";
        public event EventHandler<object>? UpdatedState;
    }

    public class TrainStatusClass
    {
        public string TrainId { get; set; }
        public int Power { get; set; }
        public double Vbat { get; set;}
    }

    public class TrainCommandClass
    {
        public string TrainId { get; set; }
        public int Power { get; set; }

        //enable the emergency brake
        public bool EStop { get; set; }

        //Power down the train
        public bool ShutDown { get; set; }

        public override string ToString()
        {
            var str = JsonSerializer.Serialize<TrainCommandClass>(this);
            return str;
        }
    }
}
