export interface HeaderProps{
    pendingTasks: number; 
    completedTasks: number;
    handleNewTaskClick(): void;
};

export interface Task{
    description: string;
    completed: boolean;
    id: number;
};

export interface TasksContainerProps{
    tasks: Task[];
    handleTaskClick(index: number): void;
    currentFilter?: Filter;
}

export type Filter = "all" | "pending" | "completed";

export interface FooterProps{
    handleFilterChange(filter: Filter): void;
}