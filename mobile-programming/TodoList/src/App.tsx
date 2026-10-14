import Header from "./components/Header";
import TasksContainer from "./components/TasksContainer";
import Footer from "./components/Footer";
import { useEffect, useState } from "react";
import { Filter, Task } from "./types";

function App(){
 
    const [pendingTasksNumber, setPendingTasksNumber] = useState(0);
    const [completedTasksNumber, setCompletedTasksNumber] = useState(0);
    const [tasksList, setTasksList] = useState<Task[]>([])
    const [filter, setFilter] = useState<Filter>("all");

    // Atualiza o estado da tarefa (completa ou pendente), os contadores e a listagem.
    function handleTaskClick(id: number): void{

        // Encontrando o índice da tarefa clicada no vetor
        const index = tasksList.findIndex(task => task.id === id);

        tasksList[index].completed = !tasksList[index].completed;

        if(tasksList[index].completed) {
            setCompletedTasksNumber(n => n + 1);
            if(pendingTasksNumber > 0) {
                setPendingTasksNumber(n => n - 1);
            }
        } else {
            setPendingTasksNumber(n => n + 1);
            if(completedTasksNumber > 0) {
                setCompletedTasksNumber(n => n - 1);
            }
        }
       
        setTasksList([...tasksList]);
    }

    // Adiciona uma nova tarefa à lista
    function handleNewTaskClick(): void{

        const inputElement = document.querySelector("#task-input") as HTMLInputElement;

        if(inputElement.value.trim() === "")
            return;
        
        const newTask: Task = { description: inputElement.value, completed: false, id: tasksList.length + 1 };

        setTasksList([...tasksList, newTask]);
        inputElement.value = "";
    }

    // Atualiza o estado do filtro
    function handleFilterChange(filter: Filter): void{
        setFilter(filter);
    }

    useEffect(()=>{

        const storedTasks = localStorage.getItem("tasks");

        if(storedTasks){

            const parsedTasks: Task[] = JSON.parse(storedTasks);
            setTasksList(parsedTasks);

            for(let task of parsedTasks){
                if(task.completed)
                    setCompletedTasksNumber(n => n + 1);
                else
                    setPendingTasksNumber(n => n + 1);
            }
        }

    },[]);

    useEffect(()=>{
        localStorage.setItem("tasks", JSON.stringify(tasksList));
    }, [tasksList]);

    return(
        <>
            <Header completedTasks={completedTasksNumber} pendingTasks={pendingTasksNumber} handleNewTaskClick={handleNewTaskClick}/>
            <TasksContainer tasks={tasksList} handleTaskClick={handleTaskClick} currentFilter={filter}/>
            <Footer handleFilterChange={handleFilterChange}/>
        </>
    )
}

export default App;