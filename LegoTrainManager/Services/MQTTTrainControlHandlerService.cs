using LegoTrainManager.Data;
using LegoTrainManager.Models;
using Microsoft.Extensions.DependencyInjection;
using MQTTnet;
using MQTTnet.Client;
using System.Diagnostics;
using JsonSerializer = System.Text.Json.JsonSerializer;

namespace LegoTrainManager.Services;

public class MQTTTrainControlHandlerService : IMQTTTrainControlHandler
{
    private readonly IServiceProvider _serviceProvider;
    private List<IMqttMessageHandler> MessageHandlers { get;  set; }= new List<IMqttMessageHandler>();
    private IConfiguration _config { get; }
    private MqttFactory _mqttFactory;
    private IMqttClient _mqttClient;


   

    public MQTTTrainControlHandlerService(IConfiguration config, IServiceProvider serviceProvider)
    {
        _serviceProvider = serviceProvider;

        _config = config;
        _mqttFactory = new MqttFactory();
        _mqttClient = _mqttFactory.CreateMqttClient();
    }

    public void AddMessageHandler(IMqttMessageHandler messageHandler)
    {
        MessageHandlers.Add(messageHandler);
    }


    public async Task EstopTrain(Train train)
    {
        var command = new TrainCommandClass
        {
            TrainId = train.Name,
            EStop = true,
            Power = 0
        };
        var topic2Send2 = GetMyTopic(train);
        await PublishAsync(topic2Send2, command.ToString());
    }

    public async Task ShutDownTrain(Train train)
    {
        var command = new TrainCommandClass
        {
            TrainId = train.Name,
            EStop = true,
            ShutDown = true
        };
        var topic2Send2 = GetMyTopic(train);
        await PublishAsync(topic2Send2, command.ToString());
    }

    public async Task UpdateTrainControl(Train train,int power)
    {

        if (train.Invert)
            power = -power;

        var trainCommand = new TrainCommandClass
        {
            TrainId = train.Name,
            Power = power
        };
        var topic2Send2 = GetMyTopic(train);
        await PublishAsync(topic2Send2, trainCommand.ToString());

    }
 
    private string GetMyTopic(Train train)
    {
        var mainTopic = _config.GetValue<string>("MQTTControlTopic");
        var topic2Send2 = mainTopic + train.Name;
        return topic2Send2;

    }
   
    private async Task ConnectAsync()
    {
        try
        {
            //using (var mqttClient = _mqttFactory.CreateMqttClient())
            //{
            // Use builder classes where possible in this project.
            var broker = _config.GetValue<string>("MQTTBroker");
            var clientId = _config.GetValue<string>("MQTTClientId");
            var mqttClientOptions = new MqttClientOptionsBuilder().
                WithTcpServer(broker).WithClientId(clientId).Build();



            _mqttClient.ApplicationMessageReceivedAsync += async e =>
            {
                using (var serviceScope = _serviceProvider.CreateScope())
                {
                    var services = serviceScope.ServiceProvider;
                    var handler = services.GetRequiredService<IMqttMessageHandler>();
                    await handler.HandleMqttMessage(e.ApplicationMessage);

                }
            };


            // This will throw an exception if the server is not available.
            // The result from this message returns additional data which was sent 
            // from the server. Please refer to the MQTT protocol specification for details.
            var response = await _mqttClient.ConnectAsync(mqttClientOptions, CancellationToken.None);

            Console.WriteLine("The MQTT client is connected.");

            await SubscribeAsync();
            //Thread.Sleep(50000);
        }
        catch (Exception e)
        {
            Console.WriteLine(e);
            throw;
        }

        

        //// Send a clean disconnect to the server by calling _DisconnectAsync_. Without this the TCP connection
        //// gets dropped and the server will handle this as a non clean disconnect (see MQTT spec for details).
        //var mqttClientDisconnectOptions = mqttFactory.CreateClientDisconnectOptionsBuilder().Build();

        //await mqttClient.DisconnectAsync(mqttClientDisconnectOptions, CancellationToken.None);
        // }
    }





    private async Task SubscribeAsync()
    {
        //var options = _mqttFactory.CreateSubscribeOptionsBuilder();

        //_messageHandlers.ForEach(m => options.WithTopicFilter(m.Topic2Subscribe2));
        //var builtOptions = options.Build();

        var mqttSubscribeOptions = _mqttFactory.CreateSubscribeOptionsBuilder()
            .WithTopicFilter(
                f =>
                {
                    f.WithTopic("Lego/Trains/Status/#");
                })
            .Build();

        await _mqttClient.SubscribeAsync(mqttSubscribeOptions, CancellationToken.None);




       // await _mqttClient.SubscribeAsync(builtOptions, CancellationToken.None);

      
    }

    private async Task PublishAsync(string topic, string msg)
    {

        if (!_mqttClient.IsConnected)
        {
            await ConnectAsync();
        }

        var applicationMessage = new MqttApplicationMessageBuilder()
            .WithTopic(topic)
            .WithPayload(msg)
            .Build();


        await _mqttClient.PublishAsync(applicationMessage, CancellationToken.None);



    }

    public async Task StartAsync(CancellationToken cancellationToken)
    {

        await ConnectAsync();

        //make all the subscriptions to the required topics;
      //await  SubscribeAsync();
    }

    public async Task StopAsync(CancellationToken cancellationToken)
    {
        await _mqttClient.DisconnectAsync();
    }
}